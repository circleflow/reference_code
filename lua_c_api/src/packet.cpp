#include "utility/export/error.h"
using namespace UTILITY;

#include "circle_flow.h"
#include "helper.h"
#include "impl.h"

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
}

#ifndef ENV_UT
#include <utility/export/smart_ptr_u.h>

#include "circle_flow/export/pkt_lib.h"
using namespace CIRCLE_FLOW;

#include <map>
using std::map;
using std::make_pair;

typedef FIELD_BLOCK (FB_MAKER) (int length);
template <FB_MAKER maker,int DFLT_LEN>
FIELD_BLOCK MAKER_LEN(int fb_len)
{
    if(0 == fb_len) {
        return maker(DFLT_LEN);
    } else {
        return maker(fb_len);
    }
}

typedef FIELD_BLOCK (FB_MAKER_FIX) (void);
template <FB_MAKER_FIX maker>
FIELD_BLOCK MAKER_FIX(int)
{
    return maker();
}

struct PROTOCOL_INFO {
    const char *  name;
    const char *  ether_type;
    FB_MAKER *fb_maker;
} ;

static
const PROTOCOL_INFO * get_protocol(const string &pro_name)
{

    static PROTOCOL_INFO protocols [] = {
            {"ipv4", "08:00", MAKER_LEN<FB_IPV4::make_IPV4, 20>},
            {"ipv6", "86:dd", MAKER_LEN<FB_IPV6::make_IPV6, 40>},
            {"track",      0, MAKER_FIX<FB_FLOW_TRACK_TAG::make_FLOW_TRACK_TAG>},
            {"vtag",       0, MAKER_FIX<FB_VLAN_TAG::make_VLAN_TAG>}
    };

    for(unsigned int i=0; i<ARRAY_NUM(protocols); i++) {
        if(0 == strcmp(protocols[i].name, pro_name.c_str())) {
            return &protocols[i];
        }
    }

    return 0;
}

typedef map< string, shared_ptr<FIELD_BLOCK> > DB_PKT;
static DB_PKT db_pkt;

static const int eth_encap_len = 16;    //dmac+smac+eth_type+fcs
static const int vtag_len = 4;

static
void pkt_create_eth_impl(const string &pkt_name,
                         const string &pro_name,
                         int length,
                         const vector<string> &tag_names)
{
    ENSURE(db_pkt.end() == db_pkt.find(pkt_name),
            "pkt \"%s\" already exist", pkt_name.c_str());

    FIELD_BLOCK pkt;
    int ntag = tag_names.size();

    if(string("raw") != pro_name) {
        const PROTOCOL_INFO *info = get_protocol(pro_name);
        ENSURE(info != 0);
        ENSURE(info->ether_type);

        length = length-eth_encap_len-vtag_len*ntag;
        ENSURE (length>0);

        FIELD_BLOCK payload(info->fb_maker(length));

        pkt.reset(FB_ETH_II::make_ETH_II(payload));
        pkt.field(FB_ETH_II::ETH_TYPE) = TEXT(info->ether_type);
    } else {
        length = length-vtag_len*ntag;
        ENSURE (length>0);

        pkt.reset(FB_ETH_II::make_ETH_II(length));
    }

    for(int i=0; i<ntag; i++) {
        FIELD_BLOCK tag(FB_VLAN_TAG::make_VLAN_TAG());
        tag.rename(tag_names[i]);
        pkt.insert(FB_ETH_II::ETH_TYPE, tag);
    }

    shared_ptr<FIELD_BLOCK> ptr(new FIELD_BLOCK(pkt));
    ptr->rename(pkt_name);

    db_pkt.insert(make_pair(pkt_name, ptr));
}

static
void pkt_create_raw_impl(const string &pkt_name,
                         const string &pro_name,
                         int length)
{
    shared_ptr<FIELD_BLOCK> ptr;

    if ("raw" != pro_name) {
        ENSURE(db_pkt.end() == db_pkt.find(pkt_name),
                "pkt \"%s\" already exist", pkt_name.c_str());

        const PROTOCOL_INFO *info = get_protocol(pro_name);
        ENSURE(info != 0);

        ptr.reset(new FIELD_BLOCK(info->fb_maker(length)));
        ptr->rename(pkt_name);
    } else {
        ptr.reset(new FIELD_BLOCK(pkt_name));
    }

    db_pkt.insert(make_pair(pkt_name, ptr));
}

FIELD_BLOCK & LUA_C_CIRCLE_FLOW::get_pkt(const string &pkt_name)
{
    DB_PKT::iterator it = db_pkt.find(pkt_name);
    ENSURE(db_pkt.end() != it,
            "pkt \"%s\" not find", pkt_name.c_str());

    return *(it->second.get());
}

static
void pkt_set_value_impl(const string &pkt_name,
                        const string &field,
                        const string &value)
{
    TRACE("%s,%s,%s\r\n", pkt_name.c_str(), field.c_str(), value.c_str());
    get_pkt(pkt_name).field(field)=value;
}

static
void pkt_set_step_impl(const string &pkt_name,
                       const string &_field,
                       bool is_incr,
                       const string &_step,
                       int cnt,
                       const string &start)
{
    FIELD &field = get_pkt(pkt_name).field(_field);
    FIELD::PARSER &parser = field.parser();

    FIELD::STEP step(is_incr,
                     parser.text_to_bytes(_step),
                     0==start.size() ? (BYTES)field : parser.text_to_bytes(start),
                     field.size_of_bit(),
                     cnt);

    field = step;
}

static
void pkt_set_random_impl(const string &pkt_name,
                         const string &_field,
                         int cnt)
{
    FIELD &field = get_pkt(pkt_name).field(_field);

    FIELD::RANDOM random(field.size_of_bit(), cnt);

    field = random;
}

static
void pkt_set_pattern_impl(const string &pkt_name,
                          const string &_field,
                          const string &_pattern)
{
    FIELD &field = get_pkt(pkt_name).field(_field);
    field.pattern(_pattern);
}

static
void pkt_destroy_impl(const string &pkt_name)
{
    DB_PKT::iterator it = db_pkt.find(pkt_name);
    ENSURE(db_pkt.end() != it,
            "pkt \"%s\" not find", pkt_name.c_str());

    db_pkt.erase(it);
}

static
shared_ptr<FIELD> make_cac_field(const string &name, lua_State *L, int index)
{
    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    string func = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    string filter = lua_tostring(L, index);

    FIELD::INDEX cac_fields;
    if(LUA_TTABLE == lua_type(L, ++index)) {

        int n = luaL_len(L, index);
        for(int i=1; i<=n; i++) {
            ENSURE(LUA_TSTRING == lua_rawgeti(L, index, i));
            cac_fields.push_back(string(lua_tostring(L, -1)));
            lua_pop(L, 1);
        }
    }

    FIELD_CACULATOR::FILTER f;
    if("black" == filter) {
        f = FIELD_CACULATOR::BLACK_LIST;
    } else if("white" == filter) {
        f = FIELD_CACULATOR::WHITE_LIST;
    } else {
        ERROR("unkown filter %s", filter.c_str());
    }

    shared_ptr<FIELD> ptr;
    if("ip_hdr_chksum" == func) {
        FIELD_CACULATOR cac(name, 16, cac_fields, f, FIELD_CACULATOR::ip_hdr_chksum);
        ptr.reset(cac.clone());
    } else if("len_byte_16b" ==  func) {
        FIELD_CACULATOR cac(name, 16, cac_fields, f, FIELD_CACULATOR::len_of_byte_16);
        ptr.reset(cac.clone());
    } else {
        ERROR("unkown func %s", func.c_str());
    }

    return ptr;
}


static
shared_ptr<FIELD> make_field(lua_State *L, int index)
{
    string name,type;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    name = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    type = lua_tostring(L, index);

    TRACE("make_field: name=%s, type=%s", name.c_str(), type.c_str());

    shared_ptr<FIELD> ptr;
    const PROTOCOL_INFO *info = get_protocol(type);

    if(info) {

        ENSURE(LUA_TNUMBER == lua_type(L, ++index));
        FIELD_BLOCK fb = info->fb_maker(lua_tonumber(L, index));
        fb.rename(name);
        ptr.reset(fb.clone());

    } else if("fixed" == type) {

        ENSURE(LUA_TNUMBER == lua_type(L, ++index));
        FIELD_FIXED_SIZE field(name, lua_tonumber(L, index));
        ptr.reset(field.clone());

    } else if ("resizable" == type) {

        ENSURE(LUA_TNUMBER == lua_type(L, ++index));
        FIELD_RESIZABLE field(name, lua_tonumber(L, index));
        ptr.reset(field.clone());

    } else if ("cac" == type) {

        return make_cac_field(name, L, index);

    } else {
        ERROR("unsupported name type %s", type.c_str());
    }

    return ptr;
}

#endif

int CIRCLE_FLOW_LUA_C::pkt_create_eth(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *pkt, *prototype;
    vector<string> tag_names;
    int length, index=0, i, ntag;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    prototype = lua_tostring(L, index);

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    length = lua_tonumber(L, index);

    ENSURE(LUA_TTABLE == lua_type(L, ++index));
    ntag = luaL_len(L, index);

    ENSURE(ntag <= 4);

    for(i=1; i<=ntag; i++) {
        ENSURE(LUA_TSTRING == lua_rawgeti(L, index, i));
        tag_names.push_back(string(lua_tostring(L, -1)));
        lua_pop(L, 1);
    }

    TRACE("pkt_create_eth:%s, prototype:%s, length:%d, num_of_tag:%d", pkt, prototype, length, ntag);
    for(i=0; i<ntag; i++) {
        TRACE(" %s", tag_names[i].c_str());
    }

#ifndef ENV_UT

    pkt_create_eth_impl(pkt, prototype, length, tag_names);

#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}


int CIRCLE_FLOW_LUA_C::pkt_create_raw(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *pkt, *prototype;
    int length, index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    prototype = lua_tostring(L, index);

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    length = lua_tonumber(L, index);

    TRACE("pkt_create_raw:%s, prototype:%s, length:%d", pkt, prototype, length);
#ifndef ENV_UT

    pkt_create_raw_impl(pkt, prototype, length);

#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::pkt_copy(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *pkt, *copy;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    copy = lua_tostring(L, index);

    TRACE("pkt_copy:%s, %s", pkt, copy);

#ifndef ENV_UT
    DB_PKT::iterator it = db_pkt.find(pkt);
    ENSURE(db_pkt.end() != it,
            "pkt \"%s\" not find", pkt);

    FIELD_BLOCK *fb = dynamic_cast<FIELD_BLOCK *>(it->second->clone());
    ENSURE(fb!=0);

    db_pkt.insert(make_pair(copy,shared_ptr<FIELD_BLOCK>(fb)));

#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::pkt_set_value(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *pkt, *field, *value;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    field = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    value = lua_tostring(L, index);

    TRACE("pkt_set_value:%s, %s=%s\r\n", pkt, field, value);

#ifndef ENV_UT

    pkt_set_value_impl(pkt, field, value);

#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::pkt_set_step(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *pkt, *field, *step, *start;
    int index=0, is_increase, cnt;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    field = lua_tostring(L, index);

    ENSURE(LUA_TBOOLEAN == lua_type(L, ++index));
    is_increase = lua_toboolean(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    step = lua_tostring(L, index);

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    cnt = lua_tonumber(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    start = lua_tostring(L, index);

    TRACE("pkt_set_step:%s, %s, is_increase=%d, step=%s, cnt=%d, start=%s\r\n",
          pkt, field, is_increase, step, cnt, start);

#ifndef ENV_UT

    pkt_set_step_impl(pkt, field, is_increase, step, cnt, start);

#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::pkt_set_random(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *pkt, *field;
    int index=0, cnt;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    field = lua_tostring(L, index);

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    cnt = lua_tonumber(L, index);

    TRACE("pkt_set_random:%s, %s, cnt=%d\r\n", pkt, field, cnt);
#ifndef ENV_UT

    pkt_set_random_impl(pkt, field, cnt);

#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::pkt_set_pattern(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *pkt, *field, *pattern;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    field = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pattern = lua_tostring(L, index);

    TRACE("pkt_set_pattern:%s, %s, pattern=%s\r\n", pkt, field, pattern);
#ifndef ENV_UT

    pkt_set_pattern_impl(pkt, field, pattern);

#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;

}

int CIRCLE_FLOW_LUA_C::pkt_show(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *pkt;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    TRACE("pkt_show:%s\r\n", pkt);
#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#else
    string str;
    if(0==strlen(pkt)) {
        for(DB_PKT::iterator it=db_pkt.begin(); it!=db_pkt.end(); it++) {
            str += it->first;
            str += " ";
        }
    } else {
        str = get_pkt(pkt).dump();
    }

    lua_pushnumber(L, 1);
    lua_pushstring(L, str.c_str());
#endif

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::pkt_get_offset_bit(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *pkt, *field;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    field = lua_tostring(L, index);

    TRACE("pkt_get_offset_bit:%s, %s\r\n", pkt, field);
#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushnumber(L, 16);
#else
    int offset = get_pkt(pkt).offset_of_bit(field);

    lua_pushnumber(L, 0);
    lua_pushnumber(L, offset);
#endif

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::pkt_destroy(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *pkt;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    TRACE("pkt_destroy:%s\r\n", pkt);
#ifndef ENV_UT
    pkt_destroy_impl(pkt);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::pkt_field_append(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *pkt;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

#ifndef ENV_UT
    get_pkt(pkt).append(*(make_field(L, index)));
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::pkt_field_insert(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *pkt, *pos;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pos = lua_tostring(L, index);

#ifndef ENV_UT
    get_pkt(pkt).insert(pos, *(make_field(L, index)));
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::pkt_field_remove(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *pkt, *field;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    field = lua_tostring(L, index);

    TRACE("pkt_field_remove:pkt=%s,field=%s", pkt, field);
#ifndef ENV_UT
    get_pkt(pkt).remove(field);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::pkt_field_extend(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *pkt, *pos,*fb;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pos = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    fb = lua_tostring(L, index);

    TRACE("pkt_field_extend:pkt=%s, pos=%s fb=%s", pkt, pos, fb);
#ifndef ENV_UT
    get_pkt(pkt).extend(pos, get_pkt(fb));
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::pkt_field_resize(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *pkt, *pos;
    int index=0, size;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pos = lua_tostring(L, index);

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    size = lua_tonumber(L, index);

    TRACE("pkt_field_resize:pkt=%s, pos=%s size=%d ", pkt, pos, size);
#ifndef ENV_UT
    FIELD_RESIZABLE &field = dynamic_cast<FIELD_RESIZABLE&>(get_pkt(pkt).field(pos));
    field.resize(size);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}
