#include "defs.h"
#include "field_impl.h"
#include "bytes_bits.h"

#include "utility/export/error.h"

#include <algorithm>
using std::remove;


FIELD::IMPL::IMPL(const FIELD::NAME &_name, UINT32 _size, const FIELD::PARSER &_parser)
: name(_name),
  size(_size),
  parser(_parser.clone()),
  parent(0)
{ }

FIELD::IMPL::IMPL(const FIELD::IMPL &ref)
: name(ref.name),
  size(ref.size),
  parser(ref.parser),
  parent(0)
{ }

void FIELD::IMPL::set_text(const TEXT &text)
{
    set_bytes(parser->text_to_bytes(text));
    cache_clear();
    seed.reset();
}

const TEXT & FIELD::IMPL::get_text(void)
{
    if(is_cacheable()) {
        if(0 == cache.text.size()) {
            cache.text = parser->bytes_to_text(get_bytes());
        }
    } else {
        cache.text = parser->bytes_to_text(get_bytes());
    }

    return cache.text;
}

void FIELD::IMPL::set_bytes(const BYTES &_bytes)
{
    set_bits(bytes_to_bits(_bytes));
    cache_clear();
    seed.reset();
}

const BYTES & FIELD::IMPL::get_bytes(void)
{
    if(is_cacheable()) {
        if(0 == cache.bytes.size()) {
            cache.bytes = bits_to_bytes(get_bits());
        }
    } else {
        cache.bytes = bits_to_bytes(get_bits());
    }

    return cache.bytes;
}

void FIELD::IMPL::set_bits(const BITS &_bits)
{
    update_bits(_bits);
    cache_clear();
    seed.reset();
}

const BITS & FIELD::IMPL::get_bits(void)
{
    if(is_cacheable()) {
        if(0 == cache.bits.size()) {
            cache.bits = fetch_bits();
        }
    } else {
        cache.bits = fetch_bits();
    }

    return cache.bits;
}

void FIELD::IMPL::set_pattern(const BYTES &pattern)
{
    UINT32 size_of_byte = (get_size_of_bit()+7)/8;
    set_bytes(pattern_fit(pattern, size_of_byte));
}

void FIELD::IMPL::set_pattern(const TEXT &pattern)
{
    set_pattern(parser->text_to_bytes(pattern));
}

void FIELD::IMPL::set_seed(const FIELD::SEED &_seed)
{
    ENSURE(size == _seed.get_size_of_bit(),
            "seed size not match with field size");
    seed = shared_ptr<FIELD::SEED> (_seed.clone());
    cache_clear();
}

void FIELD::IMPL::unset_seed()
{
    seed.reset();
}

UINT32 FIELD::IMPL::seed_cnt() const
{
    return seed.get() ? seed->get_count() : 1;
}

void FIELD::IMPL::seed_reset()
{
    if(seed.get()) {
        seed->reset();
    }
}

void FIELD::IMPL::seed_next()
{
    if(seed.get() && is_cacheable() ) {
        cache_clear();
        cache.bits = bytes_to_bits(seed->next());
        size_fit(cache.bits, size);
    }
}

void FIELD::IMPL::cache_clear()
{
    cache.bits.clear();
    cache.bytes.clear();
    cache.text.clear();

    if(parent) {
        parent->cache_clear();
    }
}

UINT32 FIELD::IMPL::get_size_of_bit()  const
{
    return size;
}

const FIELD::NAME & FIELD::IMPL::get_name() const
{
    return name;
}

void FIELD::IMPL::set_name(const FIELD::NAME &_name)
{
    name = _name;
}

FIELD::PARSER & FIELD::IMPL::get_parser()
{
    return *parser;
}

BLOCK_IMPL * FIELD::IMPL::get_parent() const
{
    return parent;
}

void FIELD::IMPL::set_parent(BLOCK_IMPL *p)
{
    parent = p;
}

static
const char * get_padding(int name_size, int align_pos)
{
    static const char padding [] = "                    ";

#define MAX_PADDING (sizeof(padding) - 1)

    const char * ptr_end = padding + MAX_PADDING;

    int padding_size = align_pos - name_size;

    if(padding_size <= 0) {
        return ptr_end;
    }

    if(padding_size >= (int)MAX_PADDING) {
        return padding;
    }

    return (ptr_end-padding_size);

}

string FIELD::IMPL::dump(UINT8 align_pos)
{

    string str(get_padding(name.size(), align_pos));

    str += name;
    str += " = ";
    str += get_text();
    str += "\r\n";

    return str;
}

FIELD::IMPL * FIELD::IMPL::get_pimpl(const FIELD *pf)
{
    ASSERT(pf);
    ASSERT(pf->pimpl);
    return pf->pimpl;
}

FIXED_SIZE_IMPL::FIXED_SIZE_IMPL(const FIELD::NAME &name, UINT32 size, const FIELD::PARSER &parser)
: FIELD::IMPL(name,size,parser),
  bits(size, false)
{ }

FIXED_SIZE_IMPL::FIXED_SIZE_IMPL(const FIXED_SIZE_IMPL &ref)
: FIELD::IMPL(ref),
  bits(ref.bits)
{ }

void FIXED_SIZE_IMPL::update_bits(const BITS &_bits)
{
    bits = _bits;
    size_fit(bits, size);
}

BITS FIXED_SIZE_IMPL::fetch_bits()
{
    return bits;
}

CACULATOR_IMPL::CACULATOR_IMPL(const FIELD::NAME &_name,
                               UINT32 _size,
                               const FIELD::INDEX &_cac_fields,
                               FIELD_CACULATOR::FILTER _filter,
                               FIELD_CACULATOR::DATA_CAC _cac_data,
                               FIELD_CACULATOR::SIZE_CAC _cac_size,
                               const FIELD::PARSER &parser)
: FIXED_SIZE_IMPL(_name, _size, parser),
  cac_fields(_cac_fields),
  filter(_filter),
  cac_data(_cac_data),
  cac_size(_cac_size)
{
    bits.clear();   //base class may assigned value

    //for data cac, avoid includes itself
    if(cac_data) {

        ENSURE((FIELD_CACULATOR::WHITE_LIST == filter) || (FIELD_CACULATOR::BLACK_LIST == filter),
                "incorrect filter type");

        FIELD::INDEX::iterator first=cac_fields.begin(), last=cac_fields.end();
        if(FIELD_CACULATOR::WHITE_LIST == filter) {
            cac_fields.erase(remove(first, last, name), last);
        } else {
            if(last == find(first, last, name)) {
                cac_fields.push_back(name);
            }
        }
    }
}

CACULATOR_IMPL::CACULATOR_IMPL(const CACULATOR_IMPL &ref)
:FIXED_SIZE_IMPL(ref),
 cac_fields(ref.cac_fields),
 filter(ref.filter),
 cac_data(ref.cac_data),
 cac_size(ref.cac_size)
{ }

class CACULATOR_IMPL::EXTRACTOR {
public:
    virtual void operator () (const FIELD &) = 0;
    virtual ~EXTRACTOR() {}
};

void CACULATOR_IMPL::traverse_cac_fields(EXTRACTOR &extractor)
{
    ENSURE(parent, "individual cac field");
    const FIELDS &fields = parent->get_fields();

    for(FIELDS::const_iterator it_field=fields.begin(); it_field!=fields.end(); it_field++) {

        bool is_find = false;
        if(cac_fields.end() != find(cac_fields.begin(),
                                    cac_fields.end(),
                                    get_pimpl(it_field->get())->get_name())) {
            is_find = true;
        }

        bool is_black_list = FIELD_CACULATOR::BLACK_LIST==filter ? true : false;

        if((is_find && (!is_black_list))
                || (!is_find && is_black_list)) {

            if((*it_field)->size_of_bit()) {
                extractor(*(*it_field));
            }
        }
    }
}

static
void accumulate_fields(const FIELD &f,BITS &bits)
{
    FIELD::IMPL *f_pimpl = FIELD::IMPL::get_pimpl(&f);

    BITS bits_add = f_pimpl->get_bits();
    bits.insert(bits.end(), bits_add.begin(), bits_add.end());
}

class EXTRACTOR_DATA : public CACULATOR_IMPL::EXTRACTOR {
public:
    void operator () (const FIELD &f)
    {
        accumulate_fields(f, bits);
    }

    BITS  bits;
};

class EXTRACTOR_SIZE : public CACULATOR_IMPL::EXTRACTOR {
public:
    EXTRACTOR_SIZE() : size(0) {}
    void operator () (const FIELD &f)
    {
        size +=f.size_of_bit();
    }

    UINT32 size;
};

BITS CACULATOR_IMPL::caculate(void)
{
    BYTES cac_bytes;

    if(parent) {

        if(cac_data) {

            EXTRACTOR_DATA extractor;
            traverse_cac_fields(extractor);

            cac_bytes = cac_data(bits_to_bytes(extractor.bits));

        } else if(cac_size) {

            EXTRACTOR_SIZE extractor;
            traverse_cac_fields(extractor);

            cac_bytes = cac_size(extractor.size);
        }
    }

    BITS cac_bits = bytes_to_bits(cac_bytes);
    size_fit(cac_bits, size);

    return cac_bits;
}

BITS CACULATOR_IMPL::fetch_bits(void)
{
    if(0 == bits.size()) {
        return caculate();
    } else {
        return bits;
    }
}

void CACULATOR_IMPL::update_bits(const BITS &_bits)
{
    if(0 == _bits.size()) {
        bits.clear();
    } else {
        FIXED_SIZE_IMPL::update_bits(_bits);
    }
}

OPTION_IMPL::OPTION_IMPL(const FIELD::NAME & name, UINT32 size, const FIELD::PARSER &parser)
: FIXED_SIZE_IMPL(name, size, parser)
{ }

OPTION_IMPL::OPTION_IMPL(const OPTION_IMPL &ref)
: FIXED_SIZE_IMPL(ref),
  options(ref.options)
{ }

void OPTION_IMPL::option_add(const BYTES &_option)
{
    BYTES option = _option;
    size_fit(option, size);

    options.insert(option);
}

bool OPTION_IMPL::is_known_option() const
{
    if(options.find(bits_to_bytes(bits)) != options.end()) {
        return true;
    } else {
        return false;
    }
}


RESIZABLE_IMPL::RESIZABLE_IMPL(const FIELD::NAME &name, UINT32 size, const FIELD::PARSER &parser)
: FIELD::IMPL(name, size, parser),
  bits(size, false)
{
    ENSURE(0 == (size%8),
            "incorrect size, must be in length of n*8 bits");
}

RESIZABLE_IMPL::RESIZABLE_IMPL(const RESIZABLE_IMPL &ref)
: FIELD::IMPL(ref),
  bits(ref.bits)
{
    ENSURE(0 == (size%8),
            "incorrect size, must be in length of n*8 bits");
}

void RESIZABLE_IMPL::update_bits(const BITS &_bits)
{
    ENSURE(0 == (_bits.size()%8),
            "incorrect size, must be in length of n*8 bits");

    size = _bits.size();
    bits=_bits;
}

BITS RESIZABLE_IMPL::fetch_bits()
{
    return bits;
}

void RESIZABLE_IMPL::resize(UINT32 size_of_byte)
{
    int n = (int)size_of_byte - (int)(size/8);

    if(0 == n) {
        return;
    }

    BYTES bytes = bits_to_bytes(bits);
    if(n > 0) {
        bytes.insert(bytes.end(), n, 0);
    } else {
        bytes.erase(bytes.end()+n, bytes.end());
    }

    bits = bytes_to_bits(bytes);
    size = size_of_byte*8;
    cache_clear();
}

void RESIZABLE_IMPL::set_seed(const FIELD::SEED &_seed)
{
    size = _seed.get_size_of_bit();
    FIELD::IMPL::set_seed(_seed);
}

BLOCK_IMPL::BLOCK_IMPL(const FIELD::NAME &name)
: FIELD::IMPL(name,0,HEX_PARSER())
{ }

BLOCK_IMPL::BLOCK_IMPL(const BLOCK_IMPL &ref)
: FIELD::IMPL(ref)
{
    for(FIELDS::const_iterator it=ref.fields.begin(); it!=ref.fields.end(); it++){
        append(it->get());
    }
}

BLOCK_IMPL::~BLOCK_IMPL()
{ }

const FIELDS & BLOCK_IMPL::get_fields(void) const
{
    return fields;
}

bool BLOCK_IMPL::is_resizable(void) const
{
    for(FIELDS::const_iterator it=fields.begin(); it!=fields.end(); it++) {
        if(get_pimpl(it->get())->is_resizable()) {
            return true;
        }
    }

    return false;
}

FIELDS::iterator BLOCK_IMPL::locate(const FIELD::NAME &_name)
{
    for(FIELDS::iterator it=fields.begin(); it!=fields.end(); it++) {

        if(_name == get_pimpl(it->get())->get_name()) {
            return it;
        }
    }

    return fields.end();
}

FIELD * BLOCK_IMPL::get_field(const FIELD::NAME &_name)
{
    FIELDS::iterator it = locate(_name);
    ENSURE(fields.end() != it,
            "field \"%s\" not found", _name.c_str());

    return it->get();
}

void BLOCK_IMPL::append (const FIELD *f)
{
    ENSURE(fields.end() == locate(get_pimpl(f)->get_name()),
            "field \"%s\" already exist", f->name().c_str());

    shared_ptr<FIELD> ptr(f->clone());
    get_pimpl(ptr.get())->set_parent(this);
    fields.push_back(ptr);

    cache_clear();
}

void BLOCK_IMPL::insert (const FIELD::NAME &_name, const FIELD *f)
{
    ENSURE(fields.end() == locate(get_pimpl(f)->get_name()),
            "field \"%s\" already exist", f->name().c_str());

    FIELDS::iterator it = locate(_name);
    ENSURE(fields.end() != it,
            "field \"%s\" not found", _name.c_str());

    shared_ptr<FIELD> ptr(f->clone());
    get_pimpl(ptr.get())->set_parent(this);
    fields.insert(it, ptr);

    cache_clear();
}

void BLOCK_IMPL::remove (const FIELD::NAME &_name)
{
    FIELDS::iterator it = locate(_name);
    ENSURE(fields.end() != it,
            "field \"%s\" not found", _name.c_str());

    fields.erase(it);

    cache_clear();

}

void BLOCK_IMPL::extend (const FIELD::NAME &_name, const FIELD_BLOCK *fb)
{
    {
        const string &fb_name = get_pimpl(fb)->get_name();
        ENSURE((fields.end()==locate(fb_name)) || (_name==fb_name),
                "field \"%s\" already exist", fb_name.c_str());
    }

    FIELDS::iterator it = locate(_name);
    ENSURE(fields.end() != it,
            "field \"%s\" not found", _name.c_str());

    shared_ptr<FIELD> ptr(fb->clone());

    {
        BLOCK_IMPL *fb_pimpl = dynamic_cast<BLOCK_IMPL *> (get_pimpl(ptr.get()));
        fb_pimpl->name = _name;
        fb_pimpl->set_parent(this);
    }

    it++;
    if(fields.end() == it) {
        remove(_name);
        fields.push_back(ptr);
    } else {
        const string &next_field = get_pimpl(it->get())->get_name();

        remove(_name);
        fields.insert(locate(next_field), ptr);
    }
}

string BLOCK_IMPL::dump(UINT8 align_pos)
{
#define BLOCK_IDENT_SPACE 2

    string str("\r\n[");
    str += name;
    str += "]\r\n";

    align_pos+=BLOCK_IDENT_SPACE;

    for(FIELDS::iterator it=fields.begin(); it!=fields.end(); it++) {

        str += FIELD::IMPL::get_pimpl(it->get())->dump(align_pos);
    }

    align_pos-=BLOCK_IDENT_SPACE;

    str += "\r\n";

    return str;
}

//total size of specified fields
static
UINT32 size_of_fields(FIELDS::const_iterator it_f,
                      FIELDS::const_iterator it_end)
{
    UINT32 size=0;
    for(FIELDS::const_iterator it=it_f; it!=it_end; it++) {

        FIELD::IMPL *f_pimpl = FIELD::IMPL::get_pimpl(it->get());

        //shall not be resizable
        ENSURE(false == f_pimpl->is_resizable(),
                "resizable field \"%s\" encounted during fields size caculation", f_pimpl->get_name().c_str());

        size += f_pimpl->get_size_of_bit();
    }

    return size;
}

void BLOCK_IMPL::update_bits(const BITS & _bits)
{
    BITS::const_iterator current = _bits.begin(), end = _bits.end();

    for(FIELDS::iterator it_field=fields.begin();
            (it_field!=fields.end()) && (current != end);
                it_field++) {

        UINT32 size_of_rest = end-current;

        FIELD::IMPL *f_pimpl = get_pimpl(it_field->get());

        //get the size of field
        UINT32 size_of_field;
        if(f_pimpl->is_resizable()) {

            //deduce the byte size of the resizable field, by the size of the fixed fields after it
            FIELDS::iterator it_next = it_field;
            it_next++;
            UINT32 size_of_after = size_of_fields(it_next, fields.end());
            ASSERT(0 == (size_of_after%8));

            ASSERT(size_of_rest >= size_of_after);
            size_of_field = size_of_rest - size_of_after;

        } else {

            size_of_field = f_pimpl->get_size_of_bit();
            ASSERT(size_of_rest >= size_of_field);
        }

        f_pimpl->set_bits(BITS(current, current+size_of_field));

        current += size_of_field;
    }
}

BITS BLOCK_IMPL::fetch_bits()
{
    BITS bits;

    for(FIELDS::iterator it=fields.begin(); it!=fields.end(); it++) {

        accumulate_fields(*(*it), bits);
    }

    return bits;
}

const PACKETS & BLOCK_IMPL::get_pkts()
{
    if(0 == pkts_cache.size()) {

        for(FIELDS::iterator it=fields.begin(); it!=fields.end(); it++) {
            get_pimpl(it->get())->seed_reset();
        }

        UINT32 num_pkt = seed_cnt();
        PACKETS pkts;

        for(UINT32 i=0; i<num_pkt; i++) {

            for(FIELDS::iterator it=fields.begin(); it!=fields.end(); it++) {
                get_pimpl(it->get())->seed_next();
            }

            pkts.push_back(get_bytes());
        }

        //cache would be cleared in every seed_next iteration, so cache at last
        pkts_cache = pkts;
    }

    return pkts_cache;
}

UINT32 BLOCK_IMPL::offset_of_bit(const FIELD::INDEX &idx)
{
    UINT32 total_offset = 0;
    FIELDS::iterator it_end = locate(idx[0]);
    ENSURE(fields.end() != it_end,
            "field \"%s\" not found", idx[0].c_str());

    for(FIELDS::iterator it=fields.begin(); it!=it_end; it++) {

        total_offset += get_pimpl(it->get())->get_size_of_bit();
    }

    if(idx.size()>1) {

        const FIELD_BLOCK *pfb = dynamic_cast<const FIELD_BLOCK *> (it_end->get());

        FIELD::INDEX _idx(idx.begin()+1, idx.end());

        total_offset += pfb->offset_of_bit(_idx);
    }

    return total_offset;
}

UINT32 BLOCK_IMPL::get_size_of_bit(void) const
{
    UINT32 total_size = 0;

    for(FIELDS::const_iterator it=fields.begin(); it!=fields.end(); it++) {

        total_size += get_pimpl(it->get())->get_size_of_bit();
    }

    return total_size;
}

unsigned int BLOCK_IMPL::seed_cnt() const
{
    UINT32 max_cnt = 0;
    for(FIELDS::const_iterator it=fields.begin(); it!=fields.end(); it++) {
        unsigned int cnt = get_pimpl(it->get())->seed_cnt();
        max_cnt = max_cnt < cnt ? cnt : max_cnt;
    }

    return max_cnt;
}

void BLOCK_IMPL::seed_reset()
{
    for(FIELDS::iterator it=fields.begin(); it!=fields.end(); it++) {
        get_pimpl(it->get())->seed_reset();
    }
}

void BLOCK_IMPL::seed_next()
{
    for(FIELDS::iterator it=fields.begin(); it!=fields.end(); it++) {
        get_pimpl(it->get())->seed_next();
    }
}

void BLOCK_IMPL::cache_clear()
{
    pkts_cache.clear();
    FIELD::IMPL::cache_clear();
}
