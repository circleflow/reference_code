
#include "defs.h"
#include "field_block.h"
#include "ethernet.h"
#include "flow_track.h"
#include "bytes_bits.h"

#include "utility/export/random_u.h"

using namespace FB_FLOW_TRACK_TAG;

#include <set>
using std::set;

/* the structure of flow track is defined in export/pkt/ethernet.h: FLOW_TRACK_TAG
 * there are few assumptions:
 * > the size of FLOW_TRACK_TAG is in unit of byte, not bit.
 * > the length of SN ID not exceed than UINT8 (8b)
 * > the length of FLOW ID, not exceed than UINT32 (32b)
 * only if changes to the structure of FLOW_TRACK_TAG break these assumptions,
 * the implementation here need to to be modified to adapt, otherwise no modification required.*/

typedef UINT32 FLOW_ID_TYPE;

UINT8 FLOW_TRACK::size(void)
{
    static UINT8 size_of_byte = 0;

    if(0 == size_of_byte){
        UINT32 len = make_FLOW_TRACK_TAG().size_of_bit();
        ASSERT(0 == len%8);

        size_of_byte = len/8;
    }

    return size_of_byte;
}

UINT8 FLOW_TRACK::max_sn(void)
{
    static UINT8 max = 0;

    if(0 == max) {
        UINT32 len = make_FLOW_TRACK_TAG().field(LATENCY_SN).size_of_bit();
        ASSERT((len<=8) && (len>0));

        UINT16 _max = (1<<len)-1;
        max = _max;
    }
    return max;
}

typedef set<FLOW_ID_TYPE> ID_SET;
static ID_SET & get_db()
{
    static ID_SET db;
    return db;
}

static
FLOW_ID_TYPE new_flow_id(UINT8 size_of_bit)
{
    FLOW_ID_TYPE mask = (1 << size_of_bit)-1;

#define FULL_THRESHOLD_FACTOR 1024

    /* prefer random than sequence,
     * random value has better chance to correctly classify a specific flow.
     *
     * assuming that the number of flow is much less than the range of ID,
     * so this method of random free ID lookup wouldn't take much retry to find a free one.
     * if the number get closing to the full range, this lookup could take much time, then we need a refactory.
     */

    //full threshold check, avoid performance degrade
    FLOW_ID_TYPE full = mask/FULL_THRESHOLD_FACTOR;
    ENSURE(get_db().size() < full);

    RAND rand;
    UINT32 key;

    key = static_cast<FLOW_ID_TYPE> (rand);
    key &= mask;

    //if not free, choose a value nearby
    while (get_db().find(key) != get_db().end()) {
        key++;
        key &= mask;
    }

    get_db().insert(key);

    return key;
}

static
void delete_flow_id(FLOW_ID_TYPE key)
{
    get_db().erase(key);
}

struct FLOW_TRACK::IMPL {
    FIELD_BLOCK track;
    FIELD_BLOCK temp_track;   //instance for efficiency purpose

    IMPL()
    :track( make_FLOW_TRACK_TAG()),
     temp_track( make_FLOW_TRACK_TAG())
    {
        UINT32 size_of_bit = track.field(FLOW_ID).size_of_bit();
        ASSERT(size_of_bit<=32);

        track.field(FLOW_ID) = make_bytes(new_flow_id(size_of_bit));
    }

    ~IMPL()
    {
        BYTES_CONVERT cvt(track.field(FLOW_ID));
        FLOW_ID_TYPE flow_id = (FLOW_ID_TYPE)cvt;

        delete_flow_id(flow_id);
    }
};


FLOW_TRACK::FLOW_TRACK(void)
{
    pimpl = new IMPL();
}

FLOW_TRACK::~FLOW_TRACK()
{
    delete pimpl;
}

BYTES FLOW_TRACK::sn_bytes(UINT8 sn) const
{
    pimpl->track.field(LATENCY_FLAG) = BYTES(1,1);
    pimpl->track.field(LATENCY_SN) = make_bytes(sn);

    return pimpl->track;
}

BYTES FLOW_TRACK::flow_bytes() const
{
    pimpl->track.field(LATENCY_FLAG) = BYTES(1,0);
    pimpl->track.field(LATENCY_SN) = BYTES(1,0);

    return pimpl->track;
}

UINT8 FLOW_TRACK::extract_sn(const BYTES &_track) const
{
    //no local instance of FIELD_BLOCK, for efficiency concern
    //not static, for re-entry concern
    pimpl->temp_track = _track;

    ENSURE((BYTES)pimpl->track.field(FLOW_ID) == (BYTES)pimpl->temp_track.field(FLOW_ID),
            "flow id not match, expected %s, actual %s",
                ((TEXT)pimpl->track.field(FLOW_ID)).c_str(),
                ((TEXT)pimpl->temp_track.field(FLOW_ID)).c_str());

    ENSURE(BYTES(1,1) == (BYTES)pimpl->temp_track.field(LATENCY_FLAG),
            "latency flag not set");

    BYTES_CONVERT sn(pimpl->temp_track.field(LATENCY_SN));
    return (UINT8)sn;
}

static
const BYTES & flow_mask(void)
{
    static BYTES mask;

    if(0==mask.size()) {

        FIELD_BLOCK track(make_FLOW_TRACK_TAG());
        track.field(FLOW_ID) = BYTES((track.field(FLOW_ID).size_of_bit()+7)/8, 0xff);

        track.field(LATENCY_FLAG) = BYTES(1,0);
        track.field(LATENCY_SN) = BYTES(1,0);

        mask = (BYTES) track;
    }

    return mask;
}

static
const BYTES & sn_mask(void)
{
    static BYTES mask;

    if(0==mask.size()) {

        FIELD_BLOCK track(make_FLOW_TRACK_TAG());
        track.field(FLOW_ID) = BYTES((track.field(FLOW_ID).size_of_bit()+7)/8, 0xff);

        track.field(LATENCY_FLAG) = BYTES(1,1);
        track.field(LATENCY_SN) = BYTES(1,0);

        mask = (BYTES) track;
    }

    return mask;
}

QUAL::MATCH FLOW_TRACK::flow_match(void) const
{
    QUAL::MATCH match;
    match.mask  = flow_mask();
    match.value = flow_bytes();

    return match;
}

QUAL::MATCH FLOW_TRACK::sn_match(void) const
{
    QUAL::MATCH match;
    match.mask  = sn_mask();
    match.value = sn_bytes(0);

    return match;
}
