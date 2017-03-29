#include "utility/export/error.h"
using namespace UTILITY;

#include "circle_flow.h"
#include "helper.h"
#include "impl.h"

#include "circle_flow/export/type.h"
#include "circle_flow/export/flow.h"
using namespace CIRCLE_FLOW;

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
}

#include <string.h>


#ifndef ENV_UT

#include <utility/export/smart_ptr_u.h>

#include <map>
using std::map;
using std::make_pair;


typedef map< string, shared_ptr<TX_FLOW> > DB_FLOW;
static DB_FLOW db_flow;

static
void tx_flow_create_impl(const string &flow_name)
{
    ENSURE(db_flow.end() == db_flow.find(flow_name),
            "flow \"%s\" already exist", flow_name.c_str());

    shared_ptr<TX_FLOW> ptr(new TX_FLOW());

    db_flow.insert(make_pair(flow_name,ptr));
}

static
void tx_flow_destroy_impl(const string &flow_name)
{
    DB_FLOW::iterator it = db_flow.find(flow_name);
    ENSURE(db_flow.end() != it,
            "flow \"%s\" not find", flow_name.c_str());

    db_flow.erase(it);
}

static
void tx_flow_set_rate_impl(const string &flow_name, const RATE &rate, const BURST &burst)
{
    DB_FLOW::iterator it = db_flow.find(flow_name);
    ENSURE(db_flow.end() != it,
            "flow \"%s\" not find", flow_name.c_str());

    it->second->set(rate, burst);
}

static
void tx_flow_set_port_impl(const string &flow_name, const PORT_NAME &port)
{
    DB_FLOW::iterator it = db_flow.find(flow_name);
    ENSURE(db_flow.end() != it,
            "flow \"%s\" not find", flow_name.c_str());

    it->second->set(port);
}

static
void tx_flow_set_pkt_impl(const string &flow_name, const string &pkt_name)
{
    DB_FLOW::iterator it = db_flow.find(flow_name);
    ENSURE(db_flow.end() != it,
            "flow \"%s\" not find", flow_name.c_str());

    FIELD_BLOCK &pkt = get_pkt(pkt_name);
    it->second->set((const PACKETS)pkt);
}

static
TX_FLOW & tx_flow_get(const string &flow_name)
{
    DB_FLOW::iterator it = db_flow.find(flow_name);
    ENSURE(db_flow.end() != it,
            "flow \"%s\" not find", flow_name.c_str());

    return *(it->second.get());
}

#endif


int CIRCLE_FLOW_LUA_C::tx_flow_create(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("tx_flow_create: %s\r\n", flow);

#ifndef ENV_UT
    tx_flow_create_impl(flow);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::tx_flow_destroy(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("tx_flow_destroy: %s\r\n", flow);

#ifndef ENV_UT
    tx_flow_destroy_impl(flow);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}


RATE LUA_C_CIRCLE_FLOW::convert_rate(const char *rate_unit, int rate_val)
{
    RATE rate;
    if(0 == strcmp(rate_unit, "kbps")) {
        rate.type = RATE::KBITS_PER_SECOND;
    } else if(0 == strcmp(rate_unit, "mbps")) {
        rate.type = RATE::MBITS_PER_SECOND;
    } else {
        ERROR("invalid rate unit");
    }

    rate.value = rate_val;

    return rate;
}

ostream & LUA_C_CIRCLE_FLOW::operator << (ostream & os, const RATE &rate)
{
    os<<rate.value;
    if(RATE::KBITS_PER_SECOND==rate.type) {
        os<<"kbps";
    } else if(RATE::MBITS_PER_SECOND==rate.type) {
        os<<"mbps";
    } else {
        ERROR("invalid rate unit");
    }

    return os;
}

int CIRCLE_FLOW_LUA_C::tx_flow_set_rate(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow, *rate_unit;
    int index=0, rate_val, burst_val;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    rate_unit = lua_tostring(L, index);

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    rate_val = lua_tonumber(L, index);

    RATE rate(convert_rate(rate_unit, rate_val));

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    burst_val = lua_tonumber(L, index);

    BURST burst(BURST::PKT, burst_val);

    TRACE("tx_flow_set_rate: %s, rate:%s, burst:%d\r\n", flow, string_any(rate).c_str(), burst.value);

#ifndef ENV_UT
    tx_flow_set_rate_impl(flow, rate, burst);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}


int CIRCLE_FLOW_LUA_C::tx_flow_set_port(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow, *port_val;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    port_val = lua_tostring(L, index);

    PORT_NAME port(port_val);

    TRACE("tx_flow_set_port: %s, port:%s\r\n", flow, port.c_str());

#ifndef ENV_UT
    tx_flow_set_port_impl(flow, port);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::tx_flow_set_pkt(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow, *pkt;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    TRACE("tx_flow_set_pkt: %s, pkt:%s\r\n", flow, pkt);

#ifndef ENV_UT
    tx_flow_set_pkt_impl(flow, pkt);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::tx_flow_start(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("tx_flow_start: %s\r\n", flow);

#ifndef ENV_UT
    tx_flow_get(flow).start();
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::tx_flow_stop(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("tx_flow_stop: %s\r\n", flow);

#ifndef ENV_UT
    tx_flow_get(flow).stop();
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

ostream & LUA_C_CIRCLE_FLOW::operator << (ostream & os, const COUNTER &cnt)
{
    os<<cnt.pkt<<" pkt / "<<cnt.byte<<" byte";
    return os;
}

ostream & LUA_C_CIRCLE_FLOW::operator << (ostream & os, const BURST &burst)
{
    os<<burst.value<<" pkt";
    return os;
}

#ifndef ENV_UT
static
string flow_summary_title()
{
    string str;

    str += left_padding("name",  10);
    str += left_padding("state", 10);
    str += left_padding("port",  10);
    str += left_padding("rate",  15);
    str += left_padding("burst", 15);
    str += "\r\n======================================================================\r\n";
    return str;
}

static
string flow_summary(const string &name, TX_FLOW &flow)
{
    string str;
    TX_FLOW::INFO info=flow.info();

    str += left_padding(name,       10);
    str += left_padding(info.is_started?"started":"stopped", 10);
    str += left_padding(info.port,  10);
    str += left_padding(info.rate,  15);
    str += left_padding(info.burst, 15);

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
string cnt_summary(const string &name, TX_FLOW &flow, bool is_clear)
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

    return str;
}

static
string cnt_detail(const string &name, TX_FLOW &flow, bool is_clear)
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

int CIRCLE_FLOW_LUA_C::tx_flow_show(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("tx_flow_show: %s\r\n", flow);

#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#else
    string str = flow_summary_title();
    if( 0==strlen(flow)) {
        for(DB_FLOW::iterator it=db_flow.begin(); it!=db_flow.end(); it++) {
            str += flow_summary(it->first, *(it->second.get()));
            str += "\r\n";
        }
    } else {
        str += flow_summary(flow, tx_flow_get(flow));
    }

    lua_pushnumber(L, 1);
    lua_pushstring(L, str.c_str());
#endif

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::tx_flow_cnt(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0, is_clear;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    ENSURE(LUA_TBOOLEAN == lua_type(L, ++index));
    is_clear = lua_toboolean(L, index);

    TRACE("tx_flow_cnt: %s, clr: %d\r\n", flow, is_clear);

#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#else
    string str;
    if( 0==strlen(flow)) {
        str = cnt_summary_title();

        for(DB_FLOW::iterator it=db_flow.begin(); it!=db_flow.end(); it++) {
            str += cnt_summary(it->first, *(it->second.get()), is_clear);
            str += "\r\n";
        }
    } else {
        str += cnt_detail(flow, tx_flow_get(flow), is_clear);
    }

    lua_pushnumber(L, 1);
    lua_pushstring(L, str.c_str());
#endif

    EXP_FREE_END_LUA_C;

    return 2;
}
