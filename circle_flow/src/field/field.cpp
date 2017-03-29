
#include "defs.h"
#include "field_block.h"
#include "field_concrete.h"
#include "type_ext.h"
#include "field_impl.h"
#include "bytes_bits.h"

FIELD::FIELD()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    pimpl = 0;

    EXP_RECAP_END;
}

FIELD::~FIELD()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    SAFE_DELETE(pimpl);

    EXP_RECAP_END;
}


void FIELD::operator = (const BYTES & bytes)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<IMPL *>(pimpl));

    pimpl->set_bytes(bytes);

    //when set a value, turns off seed
    pimpl->unset_seed();

    EXP_RECAP_END;
}


void FIELD::pattern (const BYTES & bytes)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<IMPL *>(pimpl));

    pimpl->set_pattern(bytes);

    //when set a value, turns off seed
    pimpl->unset_seed();

    EXP_RECAP_END;
}

FIELD::operator const BYTES & () const
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<IMPL *>(pimpl));

    return pimpl->get_bytes();

    EXP_RECAP_END;
}

void FIELD::operator = (const TEXT &text)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<IMPL *>(pimpl));

    pimpl->set_text(text);

    //when set a value, turns off seed
    pimpl->unset_seed();

    EXP_RECAP_END;
}

void FIELD::pattern (const TEXT & text)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<IMPL *>(pimpl));

    pimpl->set_pattern(text);

    //when set a value, turns off seed
    pimpl->unset_seed();

    EXP_RECAP_END;
}

void FIELD::operator = (const SEED &seed)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<IMPL *>(pimpl));

    pimpl->set_seed(seed);

    EXP_RECAP_END;
}

FIELD::operator const TEXT & () const
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<IMPL *>(pimpl));

    return pimpl->get_text();

    EXP_RECAP_END;
}

UINT32 FIELD::size_of_bit(void) const
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<IMPL *>(pimpl));

    return pimpl->get_size_of_bit();

    EXP_RECAP_END;

}

FIELD::NAME FIELD::name() const
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<IMPL *>(pimpl));

    return pimpl->get_name();

    EXP_RECAP_END;
}

FIELD::PARSER & FIELD::parser()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<IMPL *>(pimpl));

    return pimpl->get_parser();

    EXP_RECAP_END;
}

FIELD_FIXED_SIZE::FIELD_FIXED_SIZE()
:FIELD()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    EXP_RECAP_END;
}

FIELD_FIXED_SIZE::FIELD_FIXED_SIZE(const FIELD_FIXED_SIZE &ref)
:FIELD()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(ref.pimpl);
    ASSERT(dynamic_cast<FIXED_SIZE_IMPL *>(ref.pimpl));

    pimpl = new FIXED_SIZE_IMPL(static_cast<const FIXED_SIZE_IMPL &> (*ref.pimpl));

    EXP_RECAP_END;
}

FIELD * FIELD_FIXED_SIZE::clone(void) const
{
    return new FIELD_FIXED_SIZE(*this);
}

FIELD_FIXED_SIZE::FIELD_FIXED_SIZE(const string &name, UINT32 size, const PARSER &parser)
:FIELD()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    pimpl = new FIXED_SIZE_IMPL(name,size,parser);

    EXP_RECAP_END;
}

FIELD_FIXED_SIZE::~FIELD_FIXED_SIZE()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    SAFE_DELETE(pimpl);

    EXP_RECAP_END
}

BYTES FIELD_CACULATOR::ip_hdr_chksum(const BYTES &bytes)
{
    ENSURE(0 == (bytes.size()%2), "invalid length");

    UINT32 sum=0;

    BYTES::const_iterator it=bytes.begin();
    while(it != bytes.end()) {
        UINT16 u16 = (*it)<<8;
        it++;
        u16 |= (*it);
        it++;
        sum += u16;
    }

    while(sum>>16) {
        sum = (sum>>16) + (sum&0xffff);
    }

    UINT16 result = ~sum;
    return make_bytes(result);
}

BYTES FIELD_CACULATOR::len_of_byte_16(unsigned int size_of_bit)
{
    ENSURE(0 == (size_of_bit%8), "invalid length");
    UINT32 u32 = size_of_bit/8;
    ENSURE(u32<=65535, "inivalid length");

    return make_bytes((UINT16)u32);
}

FIELD_CACULATOR::FIELD_CACULATOR()
: FIELD_FIXED_SIZE()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    EXP_RECAP_END
}

FIELD_CACULATOR::FIELD_CACULATOR(const FIELD_CACULATOR &ref)
: FIELD_FIXED_SIZE()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(ref.pimpl);
    ASSERT(dynamic_cast<CACULATOR_IMPL *>(ref.pimpl));

    pimpl = new CACULATOR_IMPL(static_cast<const CACULATOR_IMPL &>(*ref.pimpl));

    EXP_RECAP_END
}

FIELD * FIELD_CACULATOR::clone(void) const
{
    return new FIELD_CACULATOR(*this);
}

FIELD_CACULATOR::FIELD_CACULATOR(const string & name,
                                 UINT32 size,
                                 const FIELD::INDEX &cac_fields,
                                 FILTER filter,
                                 DATA_CAC cac,
                                 const PARSER &parser)
:FIELD_FIXED_SIZE()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ENSURE(cac, "cac is null");
    ENSURE(size, "size is zero");

    pimpl = new CACULATOR_IMPL(name,size,cac_fields,filter,cac,0,parser);

    EXP_RECAP_END;
}

FIELD_CACULATOR::FIELD_CACULATOR(const string & name,
                                 UINT32 size,
                                 const FIELD::INDEX &cac_fields,
                                 FILTER filter,
                                 SIZE_CAC cac,
                                 const PARSER &parser)
:FIELD_FIXED_SIZE()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ENSURE(cac, "cac is null");
    ENSURE(size, "size is zero");

    pimpl = new CACULATOR_IMPL(name,size,cac_fields,filter,0,cac,parser);

    EXP_RECAP_END;
}

FIELD_CACULATOR::~FIELD_CACULATOR()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    SAFE_DELETE(pimpl);

    EXP_RECAP_END;
}


FIELD_OPTION::FIELD_OPTION()
: FIELD_FIXED_SIZE()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    EXP_RECAP_END;
}

FIELD_OPTION::FIELD_OPTION(const FIELD_OPTION &ref)
: FIELD_FIXED_SIZE()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(ref.pimpl);
    ASSERT(dynamic_cast<OPTION_IMPL *>(ref.pimpl));

    pimpl = new OPTION_IMPL(static_cast<const OPTION_IMPL &> (*ref.pimpl));

    EXP_RECAP_END;
}

FIELD * FIELD_OPTION::clone(void) const
{
    return new FIELD_OPTION(*this);
}

FIELD_OPTION::FIELD_OPTION(const string & name, UINT32 size, const PARSER &parser)
: FIELD_FIXED_SIZE()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    pimpl = new OPTION_IMPL(name,size,parser);

    EXP_RECAP_END;
}

FIELD_OPTION::~FIELD_OPTION()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    SAFE_DELETE(pimpl);

    EXP_RECAP_END;
}

void FIELD_OPTION::option_add(const BYTES &option)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<OPTION_IMPL *>(pimpl));

    static_cast<OPTION_IMPL *>(pimpl)->option_add(option);

    EXP_RECAP_END;
}

bool FIELD_OPTION::is_known_option(void) const
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<OPTION_IMPL *>(pimpl));

    return static_cast<OPTION_IMPL *>(pimpl)->is_known_option();

    EXP_RECAP_END;
}

FIELD_RESIZABLE::FIELD_RESIZABLE()
: FIELD()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    EXP_RECAP_END;
}

FIELD_RESIZABLE::FIELD_RESIZABLE(const FIELD_RESIZABLE &ref)
: FIELD()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(ref.pimpl);
    ASSERT(dynamic_cast<RESIZABLE_IMPL *>(ref.pimpl));

    pimpl = new RESIZABLE_IMPL(static_cast<const RESIZABLE_IMPL &> (*ref.pimpl));

    EXP_RECAP_END;
}

void FIELD_RESIZABLE::resize(unsigned int size_of_byte)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<RESIZABLE_IMPL *>(pimpl));

    static_cast<RESIZABLE_IMPL *>(pimpl)->resize(size_of_byte);

    EXP_RECAP_END;

}

FIELD * FIELD_RESIZABLE::clone(void) const
{
    return new FIELD_RESIZABLE(*this);
}

FIELD_RESIZABLE::FIELD_RESIZABLE(const string &name,
                                 UINT32 size_of_byte,
                                 const PARSER &parser)
: FIELD()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    pimpl = new RESIZABLE_IMPL(name, size_of_byte*8, parser);

    EXP_RECAP_END;
}

FIELD_RESIZABLE::~FIELD_RESIZABLE()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    SAFE_DELETE(pimpl);

    EXP_RECAP_END;
}

#include <sstream>
using std::istringstream;
using std::ostringstream;
#include <algorithm>
using std::replace;

static char name_delimiters[] = {'.', ','};

static
void scan_fields(const string &_str, vector<FIELD::NAME> &names)
{
    // replace the delimiter with blank
    string str(_str);
    for(UINT8 i=0; i<sizeof(name_delimiters); i++) {
        replace(str.begin(), str.end(), name_delimiters[i], ' ');
    }

    istringstream iss(str);

    while(!iss.eof() && !iss.fail()) {

        FIELD::NAME name;
        iss>>name;

        names.push_back(name);
    }
}

FIELD::INDEX::INDEX(const char *str)
{
    if (str) {
        scan_fields(str, *this);
    }
}

FIELD::INDEX::INDEX(const NAME &str)
{
    scan_fields(str, *this);
}

FIELD::INDEX::INDEX(vector<NAME>::const_iterator first,
                    vector<NAME>::const_iterator last)
:vector<NAME>(first, last)
{ }

FIELD_BLOCK::FIELD_BLOCK (const FIELD_BLOCK &ref)
: FIELD()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(ref.pimpl);
    ASSERT(dynamic_cast<BLOCK_IMPL *>(ref.pimpl));

    pimpl = new BLOCK_IMPL(static_cast<const BLOCK_IMPL &> (*ref.pimpl));

    EXP_RECAP_END;
}

FIELD * FIELD_BLOCK::clone(void) const
{
    return new FIELD_BLOCK(*this);
}

FIELD_BLOCK::FIELD_BLOCK(const string &name)
: FIELD()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    pimpl = new BLOCK_IMPL(name);

    EXP_RECAP_END;
}

FIELD_BLOCK::~FIELD_BLOCK()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    SAFE_DELETE(pimpl);

    EXP_RECAP_END;
}

void FIELD_BLOCK::rename(const NAME &name)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<BLOCK_IMPL *>(pimpl));

    static_cast<BLOCK_IMPL *>(pimpl)->set_name(name);

    EXP_RECAP_END;
}

FIELD_BLOCK & FIELD_BLOCK::block (const FIELD::INDEX &idx)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<BLOCK_IMPL *>(pimpl));

    UINT32 num_of_name = idx.size();
    ENSURE(num_of_name>=1, "empty name");

    if(num_of_name>1) {
        FIELD_BLOCK & fb = this->block(FIELD::INDEX(idx[0]));
        return fb.block( FIELD::INDEX(idx.begin()+1, idx.end()) );
    } else {
        FIELD *f = (static_cast<BLOCK_IMPL *>(pimpl))->get_field(idx[0]);
        return *(dynamic_cast<FIELD_BLOCK *> (f));
    }

    EXP_RECAP_END;
}

const FIELD_BLOCK & FIELD_BLOCK::block (const FIELD::INDEX &idx) const
{
    return const_cast<FIELD_BLOCK *>(this)->block (idx);
}


FIELD & FIELD_BLOCK::field (const FIELD::INDEX &idx)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<BLOCK_IMPL *>(pimpl));

    UINT32 num_of_name = idx.size();
    ENSURE(num_of_name, "empty name");

    if(num_of_name > 1) {
        FIELD_BLOCK &fb = this->block(FIELD::INDEX(idx[0]));
        return fb.field( FIELD::INDEX(idx.begin()+1, idx.end()) );
    } else {
        FIELD *pf = static_cast<BLOCK_IMPL *>(pimpl)->get_field(idx[0]);
        return *pf;
    }

    EXP_RECAP_END;
}

const FIELD & FIELD_BLOCK::field (const FIELD::INDEX &idx) const
{
    return const_cast<FIELD_BLOCK *>(this)->field(idx);
}

void FIELD_BLOCK::append (const FIELD &f)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<BLOCK_IMPL *>(pimpl));

    static_cast<BLOCK_IMPL *>(pimpl)->append(&f);

    EXP_RECAP_END;
}

void FIELD_BLOCK::insert (const FIELD::INDEX &idx, const FIELD &f)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<BLOCK_IMPL *>(pimpl));

    UINT32 num_of_name = idx.size();
    ENSURE(num_of_name, "empty name");

    if(num_of_name > 1) {
        FIELD_BLOCK &fb = this->block(FIELD::INDEX(idx[0]));
        fb.insert( FIELD::INDEX(idx.begin()+1, idx.end()), f);
    } else {
        static_cast<BLOCK_IMPL *>(pimpl)->insert(idx[0], &f);
    }

    EXP_RECAP_END;
}

void FIELD_BLOCK::remove (const FIELD::INDEX &idx)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<BLOCK_IMPL *>(pimpl));

    UINT32 num_of_name = idx.size();
    ENSURE(num_of_name, "empty name");

    if(num_of_name > 1) {
        FIELD_BLOCK &fb = this->block(FIELD::INDEX(idx[0]));
        fb.remove( FIELD::INDEX(idx.begin()+1, idx.end()));
    } else {
        static_cast<BLOCK_IMPL *>(pimpl)->remove(idx[0]);
    }

    EXP_RECAP_END;
}

void FIELD_BLOCK::extend(const FIELD::INDEX &idx, const FIELD_BLOCK &fb_ext)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<BLOCK_IMPL *>(pimpl));

    UINT32 num_of_name = idx.size();
    ENSURE(num_of_name, "empty name");

    if(num_of_name > 1) {
        FIELD_BLOCK &fb = this->block(FIELD::INDEX(idx[0]));
        fb.extend( FIELD::INDEX(idx.begin()+1, idx.end()), fb_ext);
    } else {
        static_cast<BLOCK_IMPL *>(pimpl)->extend(idx[0], &fb_ext);
    }

    EXP_RECAP_END;
}

int FIELD_BLOCK::offset_of_bit(const FIELD::INDEX &idx) const
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    FIELD_BLOCK *pfb = const_cast<FIELD_BLOCK *>(this);

    ASSERT(pfb->pimpl);
    ASSERT(dynamic_cast<BLOCK_IMPL *>(pimpl));

    return static_cast<BLOCK_IMPL *>(pfb->pimpl)->offset_of_bit(idx);

    EXP_RECAP_END;
}

void FIELD_BLOCK::reset(const FIELD_BLOCK &ref)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<BLOCK_IMPL *>(pimpl));
    delete pimpl;

    ASSERT(ref.pimpl);
    ASSERT(dynamic_cast<BLOCK_IMPL *>(ref.pimpl));

    pimpl = new BLOCK_IMPL(static_cast<const BLOCK_IMPL &> (*ref.pimpl));

    EXP_RECAP_END;
}

string FIELD_BLOCK::dump() const
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);
    ASSERT(dynamic_cast<BLOCK_IMPL *>(pimpl));

    return static_cast<BLOCK_IMPL *>(pimpl)->dump();

    EXP_RECAP_END;
}

FIELD_BLOCK::operator const PACKETS & () const
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    FIELD_BLOCK *pfb = const_cast<FIELD_BLOCK *>(this);

    ASSERT(pfb->pimpl);
    ASSERT(dynamic_cast<BLOCK_IMPL *>(pfb->pimpl));

    return static_cast<BLOCK_IMPL *>(pfb->pimpl)->get_pkts();

    EXP_RECAP_END;
}


