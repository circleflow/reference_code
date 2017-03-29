
#ifndef FIELD_IMPL_H_
#define FIELD_IMPL_H_

#include "utility/export/smart_ptr_u.h"
#include "utility/export/error.h"
using namespace UTILITY;

#include "field_concrete.h"
#include "type_ext.h"
#include "field_seed.h"
#include "field_parser.h"



#include <set>
using std::set;

typedef list< shared_ptr<FIELD> > FIELDS;

class BLOCK_IMPL;

/* value of field saved in the derived class of FIELD::IMPL
 * cached text/bytes/bits, saved in FIELD::IMPL
 * */

class CIRCLE_FLOW::FIELD::IMPL {

public:
    IMPL(const FIELD::NAME &name, UINT32 size_of_bit, const FIELD::PARSER &parser);
    IMPL(const FIELD::IMPL &);
    virtual ~IMPL() {}

    void         set_text(const TEXT &text);
    const TEXT & get_text(void);

    void          set_bytes(const BYTES &bytes);
    const BYTES & get_bytes(void);

    void         set_bits(const BITS &bits);
    const BITS & get_bits(void);

    void set_pattern(const BYTES &pattern);
    void set_pattern(const TEXT  &pattern);

    virtual void   set_seed(const FIELD::SEED &seed);
    virtual void   unset_seed();
    virtual UINT32 seed_cnt() const;
    virtual void   seed_reset();
    virtual void   seed_next();

    virtual UINT32 get_size_of_bit()  const;

    const FIELD::NAME & get_name() const;
    void                set_name(const FIELD::NAME &name);

    FIELD::PARSER & get_parser();

    BLOCK_IMPL * get_parent() const;
    void         set_parent(BLOCK_IMPL *);

    virtual bool is_resizable() const = 0;

    virtual string dump(UINT8 align_pos);

    static IMPL *  get_pimpl(const FIELD *pf);

protected:
    FIELD::NAME name;
    UINT32 size;
    shared_ptr<FIELD::SEED> seed;
    shared_ptr<FIELD::PARSER> parser;
    BLOCK_IMPL *parent;

    virtual void update_bits(const BITS &_bits) = 0;
    virtual BITS fetch_bits() = 0;

    virtual bool is_cacheable() const = 0;

    virtual void cache_clear();

    struct CACHE {
        BITS   bits;
        BYTES  bytes;
        TEXT   text;
    } cache;

};

class FIXED_SIZE_IMPL : public FIELD::IMPL {
public:

    FIXED_SIZE_IMPL(const FIELD::NAME &name, UINT32 size, const FIELD::PARSER &parser);
    FIXED_SIZE_IMPL(const FIXED_SIZE_IMPL &);

protected:
    BITS bits;

    void update_bits(const BITS &_bits);
    BITS fetch_bits();

    bool is_resizable() const       { return false; }
    bool is_cacheable() const       { return true;  }
};

class CACULATOR_IMPL : public FIXED_SIZE_IMPL {

    FIELD::INDEX cac_fields;
    FIELD_CACULATOR::FILTER filter;
    FIELD_CACULATOR::DATA_CAC cac_data;
    FIELD_CACULATOR::SIZE_CAC cac_size;

public:
    CACULATOR_IMPL(const string &name,
                   UINT32 size,
                   const FIELD::INDEX &cac_fields,
                   FIELD_CACULATOR::FILTER filter,
                   FIELD_CACULATOR::DATA_CAC cac_data,
                   FIELD_CACULATOR::SIZE_CAC cac_size,
                   const FIELD::PARSER &parser);

    CACULATOR_IMPL(const CACULATOR_IMPL &);

    class EXTRACTOR;

protected:
    void update_bits(const BITS &_bits);
    BITS fetch_bits();

    bool is_cacheable() const           { return false;  }

    void set_seed(const FIELD::SEED &)  { ERROR("invalid"); }

    void traverse_cac_fields(EXTRACTOR &extractor);
    BITS caculate(void);
};

class OPTION_IMPL : public FIXED_SIZE_IMPL {

    set<BYTES> options;

public:
    OPTION_IMPL(const FIELD::NAME & name, UINT32 size, const FIELD::PARSER &parser);
    OPTION_IMPL(const OPTION_IMPL &);

    void option_add(const BYTES &option);
    bool is_known_option() const;

};

class RESIZABLE_IMPL : public FIELD::IMPL {
public:
    RESIZABLE_IMPL(const FIELD::NAME &name, UINT32 size, const FIELD::PARSER &parser);
    RESIZABLE_IMPL(const RESIZABLE_IMPL &);

    void set_seed(const FIELD::SEED &seed);

    void resize(UINT32 size_of_byte);

protected:
    BITS bits;

    bool is_resizable(void) const   { return true; }
    bool is_cacheable() const       { return true; }

    void update_bits(const BITS & bits);
    BITS fetch_bits();
};


class BLOCK_IMPL : public FIELD::IMPL {
public:
    BLOCK_IMPL(const FIELD::NAME &name);
    BLOCK_IMPL(const BLOCK_IMPL &);
    ~BLOCK_IMPL();

    const FIELDS & get_fields(void) const;

    FIELDS::iterator locate(const FIELD::NAME &_name);
    FIELD * get_field(const FIELD::NAME &_name);

    void append (const FIELD *f);
    void insert (const FIELD::NAME &_name, const FIELD *f);
    void remove (const FIELD::NAME &_name);
    void extend (const FIELD::NAME &_name, const FIELD_BLOCK *fb);

    void   set_seed(const FIELD::SEED &)    { ERROR("invalid"); }
    UINT32 seed_cnt() const;
    void   seed_reset();
    void   seed_next();

    const PACKETS & get_pkts();

    UINT32 offset_of_bit(const FIELD::INDEX &idx);
    UINT32 get_size_of_bit(void) const;

    string dump(UINT8 align_pos=14);

    void cache_clear();

protected:
    FIELDS fields;
    PACKETS pkts_cache;

    void update_bits(const BITS &bits);
    BITS fetch_bits();

    bool is_resizable() const;
    bool is_cacheable() const   { return true; }

};
#endif /* FIELD::IMPL_H_ */
