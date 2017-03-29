
#include "udf.h"
#include "hal_udf.h"

#include "utility/export/smart_ptr_u.h"
#include "utility/export/error.h"
#include "utility/export/unique.h"
#include <map>
using std::multimap;
using std::make_pair;
using std::pair;

/* UDF(user defined field)
 * is used for RX qualify when the predefined field could not meet the requirement
 * FP RX group has to specify the UDF during its creation,
 * but we can not know the proper UDF setting until RX qualify created.
 * we may need many UDF depends how many RX qualify includes UDF,
 * besides, RX latency also need UDF to locate latency tag.
 * so the solution turns to be:
 * > predefine all UDF via SDK, use it for FP RX group
 * > all qualify share the same UDF, with different mask (select different chunks)
 * > manage all chunks by ourself(direct hw touch):choose chunk for each UDF window
 */

/* ============== HW UDF resource ================== */

/* UDF HW specs abstract the difference between HW chip
 * so that the algorithm of chunk selecting can be unified to most chip */

#define CHUNK_NUM  hal_udf_specs().num_of_chunk
#define CHUNK_SIZE hal_udf_specs().len_of_chunk
#define MAX_OFFSET hal_udf_specs().max_offset
#define MAX_LENGTH (hal_udf_specs().num_of_chunk*hal_udf_specs().len_of_chunk)
#define ALIGNED_OFFSET(offset)      hal_udf_specs().chunk_aligned_offset(offset)

//as a template argument, it must has external linkage
namespace CIRCLE_FLOW {
    UINT8 _udf_max_id() { return CHUNK_NUM-1;  }
}

struct HW_CHUNK {
    int unit;
    int offset;

    //udf window index allocation per unit, unit id as pool index
    UNIQUE<HW_CHUNK, UINT8, 0, 0, unique_id_val<UINT8,0>, _udf_max_id> idx;

    HW_CHUNK( int _unit, int _offset)
    :unit(_unit),
     offset(_offset),
     idx(unit)
    {
        hal_udf_chunk_set(unit, idx.get_id(), offset);
    }
};

#define HW_CHUNK_KEY(unit,offset) make_pair(unit,offset)

typedef multimap< pair<int,int>, shared_ptr<HW_CHUNK> > DB_HW_CHUNK;
static DB_HW_CHUNK db_hw_chunk;

/* ============== SW UDF class ================== */

struct CHUNK {
    int   base_offset;
    UINT8 relative_offset;
    UINT8 length;
};

static
void split_to_chunk(UINT8 offset, UINT8 length, vector<CHUNK> &chunks)
{
    CHUNK chunk;

    chunk.base_offset = ALIGNED_OFFSET(offset);
    chunk.relative_offset = offset - chunk.base_offset;
    if((length + chunk.relative_offset)>CHUNK_SIZE) {

        chunk.length = CHUNK_SIZE - chunk.relative_offset;
        chunks.push_back(chunk);

        length -= chunk.length;
        offset = chunk.base_offset + CHUNK_SIZE;

        split_to_chunk(offset, length, chunks);

    } else {
        chunk.length = length;
        chunks.push_back(chunk);
    }
}

struct UDF::IMPL {
    IMPL(UINT8 _unit, UINT8 _length, const vector<CHUNK> &_chunks)
    :unit(_unit), length(_length), chunks(_chunks)
    {
        for(UINT8 i=0; i<chunks.size(); i++) {
            int offset = chunks[i].base_offset;
            DB_HW_CHUNK::iterator it = db_hw_chunk.find(HW_CHUNK_KEY(unit,offset));

            if(it == db_hw_chunk.end()) {
                shared_ptr<HW_CHUNK> ptr(new HW_CHUNK(unit, offset));
                db_hw_chunk.insert(make_pair(HW_CHUNK_KEY(unit,offset), ptr));
            } else {
                db_hw_chunk.insert(make_pair(HW_CHUNK_KEY(unit,offset), it->second));
            }
        }
    }

    ~IMPL()
    {
        for(UINT8 i=0; i<chunks.size(); i++) {

            int offset = chunks[i].base_offset;

            DB_HW_CHUNK::iterator it = db_hw_chunk.find(HW_CHUNK_KEY(unit,offset));
            ASSERT(it != db_hw_chunk.end());

            db_hw_chunk.erase(it);
        }
    }

    void set(const vector<UINT8> &_data, const vector<UINT8> &_mask)
    {
        ENSURE(_data.size() == length, "unmatched length");
        ENSURE(_mask.size() == length, "unmatched length");

        data = _data;
        mask = _mask;
    }

    /*convert a logical view of UDF to physical view of UDF
      physical view contains all hw chunks, so the "match" should mask off the unrelated chunks
    */
    QUAL::MATCH get(void) const
    {
        QUAL::MATCH match;

        match.value.resize(CHUNK_SIZE*CHUNK_NUM, 0);
        match.mask = match.value;

        if(0 == length) {
            return match;
        }

        vector<UINT8>::const_iterator src_d = data.begin();
        vector<UINT8>::const_iterator src_m = mask.begin();

        vector<UINT8>::iterator dst_d = match.value.begin();
        vector<UINT8>::iterator dst_m = match.mask.begin();

        for(UINT8 i=0; i<chunks.size(); i++) {

            DB_HW_CHUNK::iterator it=db_hw_chunk.find(HW_CHUNK_KEY(unit,chunks[i].base_offset));
            ASSERT(it != db_hw_chunk.end());
            UINT8 hw_chunk_idx = it->second->idx.get_id();

            UINT8 offset = hw_chunk_idx*CHUNK_SIZE + chunks[i].relative_offset;
            UINT8 len = chunks[i].length;

            copy(src_d, src_d+len, dst_d+offset);
            copy(src_m, src_m+len, dst_m+offset);

            src_d += len;
            src_m += len;
        }

        return match;
    }

    UINT8 unit;
    UINT8 length;
    vector<CHUNK> chunks;

    vector<UINT8> data;
    vector<UINT8> mask;

private:
    IMPL();
    IMPL(const IMPL &);
    IMPL & operator = (const IMPL &);
};

UDF::UDF (UINT8 unit, UINT8 offset, UINT8 length)
{
    if(offset > MAX_OFFSET) {
        ERROR("invalid offset");
    }

    if(length > MAX_LENGTH) {
        ERROR("invalid length");
    }

    if((offset + length) > (MAX_OFFSET+1)) {
        ERROR("invalid offset + length");
    }

    vector<CHUNK> chunks;
    split_to_chunk(offset, length, chunks);

    if(chunks.size() > CHUNK_NUM) {
        ERROR("cost too many chunks");
    }

    pimpl = new IMPL(unit, length, chunks);
}

UDF::~UDF()
{
    delete pimpl;
}

void UDF::set_match(const QUAL::MATCH &match)
{
    pimpl->set(match.value, match.mask);
}

QUAL::MATCH UDF::get_match(void) const
{
    return pimpl->get();
}
