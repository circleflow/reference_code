#include "utility/export/error.h"
using namespace UTILITY;

#include "circle_flow.h"
#include "helper.h"
#include "impl.h"

#include "circle_flow/export/type.h"
#include "circle_flow/export/flow.h"
#include "circle_flow/export/field/field_parser.h"

using namespace CIRCLE_FLOW;
using namespace QUAL_HELPER;

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
}

#include <string.h>

ostream & LUA_C_CIRCLE_FLOW::operator << (ostream & os, const PORT_NAME_SET &ports)
{
    for(PORT_NAME_SET::const_iterator it=ports.begin(); it!=ports.end(); it++) {
        os<<*it<<" ";
    }
    return os;
}

static
ostream & operator << (ostream & os, const QUAL::UDF_RULES &udfs)
{
    HEX_PARSER parser;

    for(QUAL::UDF_RULES::const_iterator it=udfs.begin(); it!=udfs.end(); it++) {
        os<<"udf(offset="<<(int)it->first.offset<<",size="<<(int)it->first.size
                <<")="<<parser.bytes_to_text(it->second.value)<<"/"<<parser.bytes_to_text(it->second.mask)
                <<"\r\n";
    }
    return os;
}

static
ostream & operator << (ostream & os, const QUAL::PDF_RULES &pdfs)
{
    HEX_PARSER parser;

    for(QUAL::PDF_RULES::const_iterator it=pdfs.begin(); it!=pdfs.end(); it++) {
        PDF_INFO info = get_pdf_info(it->first);
        os<<info.name<<"="<<parser.bytes_to_text(it->second.value)<<"/"<<parser.bytes_to_text(it->second.mask)
                <<"\r\n";
    }
    return os;
}


#ifndef ENV_UT

#include <utility/export/smart_ptr_u.h>

#include <map>
using std::map;
using std::make_pair;


typedef map< string, shared_ptr<RX_FLOW> > DB_FLOW;
static DB_FLOW db_flow;

static
RX_FLOW & rx_flow_get(const string &flow_name)
{
    DB_FLOW::iterator it = db_flow.find(flow_name);
    ENSURE(db_flow.end() != it,
            "flow \"%s\" not find", flow_name.c_str());

    return *(it->second.get());
}

static
void rx_flow_create_impl(const string &flow_name)
{
    ENSURE(db_flow.end() == db_flow.find(flow_name),
            "flow \"%s\" already exist", flow_name.c_str());

    shared_ptr<RX_FLOW> ptr(new RX_FLOW());

    db_flow.insert(make_pair(flow_name,ptr));
}

static
void rx_flow_destroy_impl(const string &flow_name)
{
    DB_FLOW::iterator it = db_flow.find(flow_name);
    ENSURE(db_flow.end() != it,
            "flow \"%s\" not find", flow_name.c_str());

    db_flow.erase(it);
}

bool is_zero(const BYTES &bytes)
{
    UINT8 u8=0;
    for(BYTES::const_iterator it=bytes.begin(); it!=bytes.end(); it++) {
        u8 |= *it;
    }

    return u8==0 ? true : false;
}

QUAL qual_merge(const QUAL &base, const QUAL &incr)
{
    QUAL qual=base;

    if(incr.in_ports.size()>0) {
        qual.in_ports = incr.in_ports;
    }

    for(QUAL::PDF_RULES::const_iterator it=incr.pdfs.begin(); it!=incr.pdfs.end(); it++) {
        if(is_zero(it->second.mask)) {
            //delete item
            if(qual.pdfs.end() != qual.pdfs.find(it->first)) {
                qual.pdfs.erase(it->first);
            }
        } else {
            qual.pdfs[it->first] = it->second;
        }
    }

    for(QUAL::UDF_RULES::const_iterator it=incr.udfs.begin(); it!=incr.udfs.end(); it++){
        if(is_zero(it->second.mask)) {
            //delete item
            if(qual.udfs.end() != qual.udfs.find(it->first)) {
                qual.udfs.erase(it->first);
            }
        } else {
            qual.udfs[it->first] = it->second;
        }
    }

    return qual;
}

static
void rx_flow_set_impl(const string &flow_name, const QUAL &incr)
{
    RX_FLOW &flow = rx_flow_get(flow_name);

    RX_FLOW::INFO info = flow.info();

    QUAL qual = qual_merge(info.qual, incr);

    flow.set(qual);

}

#endif


int CIRCLE_FLOW_LUA_C::rx_flow_create(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("rx_flow_create: %s\r\n", flow);

#ifndef ENV_UT
    rx_flow_create_impl(flow);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::rx_flow_destroy(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("rx_flow_destroy: %s\r\n", flow);

#ifndef ENV_UT
    rx_flow_destroy_impl(flow);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::rx_flow_set_port(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;
    QUAL qual;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    {
        int nport;
        ENSURE(LUA_TTABLE == lua_type(L, ++index));
        nport = luaL_len(L, index);

        for(int i=1; i<=nport; i++) {
            ENSURE(LUA_TSTRING == lua_rawgeti(L, index, i));
            qual.in_ports.insert(string(lua_tostring(L, -1)));
            lua_pop(L, 1);
        }
    }

    TRACE("rx_flow_set_port: %s, %s \r\n", flow, string_any(qual.in_ports).c_str());

#ifndef ENV_UT
    rx_flow_set_impl(flow, qual);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}


int CIRCLE_FLOW_LUA_C::rx_flow_set_udf(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;
    QUAL qual;

    int offset,size;
    const char * value, *mask;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    offset = lua_tonumber(L, index);

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    size = lua_tonumber(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    value = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    mask = lua_tostring(L, index);

    qual.udfs.insert(QUAL_HELPER::make_udf_rule(offset, size, value, mask));

    TRACE("rx_flow_set_udf: %s, %s", flow, string_any(qual.udfs).c_str());

#ifndef ENV_UT
    rx_flow_set_impl(flow, qual);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}


int CIRCLE_FLOW_LUA_C::rx_flow_set_udf_prefix(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;
    QUAL qual;

    int offset,size,prefix;
    const char * value;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    offset = lua_tonumber(L, index);

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    size = lua_tonumber(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    value = lua_tostring(L, index);

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    prefix = lua_tonumber(L, index);

    qual.udfs.insert(QUAL_HELPER::make_udf_rule(offset, size, value, prefix));

    TRACE("rx_flow_set_udf_prefix: %s, %s", flow, string_any(qual.udfs).c_str());

#ifndef ENV_UT
    rx_flow_set_impl(flow, qual);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::rx_flow_set_pdf(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;
    QUAL qual;

    const char *field, *value, *mask;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    field = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    value = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    mask = lua_tostring(L, index);

    qual.pdfs.insert(QUAL_HELPER::make_pdf_rule(field, value, mask));

    TRACE("rx_flow_set_pdf: %s, %s",
            flow,
            string_any(qual.pdfs).c_str());

#ifndef ENV_UT
    rx_flow_set_impl(flow, qual);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}


int CIRCLE_FLOW_LUA_C::rx_flow_set_pdf_prefix(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;
    QUAL qual;

    int prefix;
    const char *field, *value;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    field = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    value = lua_tostring(L, index);

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    prefix = lua_tonumber(L, index);

    qual.pdfs.insert(QUAL_HELPER::make_pdf_rule(field, value, prefix));

    TRACE("rx_flow_set_pdf_prefix: %s, %s",
            flow,
            string_any(qual.pdfs).c_str());

#ifndef ENV_UT
    rx_flow_set_impl(flow, qual);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}
int CIRCLE_FLOW_LUA_C::rx_flow_start(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("rx_flow_start: %s\r\n", flow);

#ifndef ENV_UT
    rx_flow_get(flow).start();
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::rx_flow_stop(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("rx_flow_stop: %s\r\n", flow);

#ifndef ENV_UT
    rx_flow_get(flow).stop();
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

#ifndef ENV_UT
static
const string & flow_summary_title()
{
    static string str;
    static bool run_once = true;

    if(run_once) {
        str += left_padding("  name",     10);
        str += left_padding("  status",   10);
        str += left_padding("  in_ports", 12);
        str += "\r\n";

        run_once = false;
    }

    return str;
}

static
string flow_summary(const string &name, RX_FLOW &flow)
{
    RX_FLOW::INFO info=flow.info();

    string str;
    str += left_padding(name,       10);
    str += left_padding(info.is_started?
                            (info.is_snooping?"snooping":"started")
                            :"stopped", 10);

    str += "  ";
    str += string_any(info.qual.in_ports);

    str += "\r\n";

    return str;
}

static
string flow_detail(const string &name, RX_FLOW &flow)
{
    string str = flow_summary_title();
    str += flow_summary(name, flow);

    RX_FLOW::INFO info=flow.info();

    str += "\r\n ==== quals ====\r\n";
    str += string_any(info.qual.pdfs);
    str += string_any(info.qual.udfs);

    return str;
}

static
string cnt_summary_title()
{
    string str;

    str += left_padding("name",  10);
    str += left_padding("pkt",   16);
    str += left_padding("byte",  16);
    str += left_padding("pkt/s", 16);
    str += left_padding("byte/s",16);
    str += "\r\n===================================================================================\r\n";
    return str;

}

static
string cnt_summary(const string &name, RX_FLOW &flow, bool is_clear)
{
    COUNTER cnt  = flow.counter(is_clear);
    COUNTER rate = flow.counter_rate();

    string str;

    str += left_padding(name,     10);
    str += left_padding(cnt.pkt,  15);
    str += " ";
    str += left_padding(cnt.byte, 15);
    str += " ";
    str += left_padding(rate.pkt, 15);
    str += " ";
    str += left_padding(rate.byte,15);
    str += "\r\n";

    return str;
}

static
string cnt_detail(const string &name, RX_FLOW &flow, bool is_clear)
{
    COUNTER cnt  = flow.counter(is_clear);
    COUNTER rate = flow.counter_rate();

    string str;

    str += " ==== " + name + " ====\r\n";
    str += "\r\n pkt   : " + string_any(cnt.pkt);
    str += "\r\n byte  : " + string_any(cnt.byte);
    str += "\r\n pkt/s : " + string_any(rate.pkt);
    str += "\r\n byte/s: " + string_any(rate.byte);

    return str;
}

#endif

int CIRCLE_FLOW_LUA_C::rx_flow_show(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("rx_flow_show: %s.\r\n", flow);

#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#else
    string str;
    if(0 == strlen(flow)) {
        str = flow_summary_title();
        for(DB_FLOW::iterator it=db_flow.begin(); it!=db_flow.end(); it++) {
            str += flow_summary(it->first, *(it->second.get()));
        }
    } else {
        str = flow_detail(flow, rx_flow_get(flow));
    }

    lua_pushnumber(L, 1);
    lua_pushstring(L, str.c_str());
#endif


    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::rx_flow_cnt(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0, is_clear;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    ENSURE(LUA_TBOOLEAN == lua_type(L, ++index));
    is_clear = lua_toboolean(L, index);

    TRACE("rx_flow_cnt: %s, clr: %d\r\n", flow, is_clear);

#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#else
    string str;
    if(0 == strlen(flow)) {
        str = cnt_summary_title();
        for(DB_FLOW::iterator it=db_flow.begin(); it!=db_flow.end(); it++) {
            str += cnt_summary(it->first, *(it->second.get()), is_clear);
        }
    } else {
        str = cnt_detail(flow, rx_flow_get(flow), is_clear);
    }

    lua_pushnumber(L, 1);
    lua_pushstring(L, str.c_str());
#endif


    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::rx_flow_snoop(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0, max_cnt;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    max_cnt = lua_tonumber(L, index);

    TRACE("rx_flow_snoop: %s, max=%d\r\n", flow, max_cnt);

#ifndef ENV_UT
    rx_flow_get(flow).snoop(max_cnt);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

ostream & LUA_C_CIRCLE_FLOW::operator << (ostream & os, const SNOOP_PKT &snoop)
{
    os<<"\r\n==============================================\r\n"
      <<"  timestamp:"<<left_padding(snoop.time_stamp,12)
      <<" size: "<<left_padding(snoop.pkt.size(),12)<<endl;

    os.setf (std::ios::hex , std::ios::basefield);
    os.setf (std::ios::right , std::ios::adjustfield);
    os.unsetf (std::ios::showbase);

    UINT8 i=0;
    UINT32 max = snoop.pkt.size()-1;
    for(; i<max; i++) {

        os.width(2);
        os.fill('0');

        os<<(short)snoop.pkt[i];

        if(15 == (i%16)) {
            os<<endl;
        } else {
            os<<":";
        }
    }

    os.width(2);
    os.fill('0');

    os<<(unsigned short)snoop.pkt[i];

    return os;
}

int CIRCLE_FLOW_LUA_C::rx_flow_dump(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("rx_flow_dump: %s\r\n", flow);

#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#else
    SNOOP_PKTS snoops;
    rx_flow_get(flow).dump(snoops);

    string str;
    for(SNOOP_PKTS::const_iterator it=snoops.begin(); it!=snoops.end(); it++) {
      str += string_any(*it);
    }

    lua_pushnumber(L, 1);
    lua_pushstring(L, str.c_str());
#endif


    EXP_FREE_END_LUA_C;

    return 2;
}
