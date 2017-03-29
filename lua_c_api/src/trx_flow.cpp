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


#ifndef ENV_UT

#include <utility/export/smart_ptr_u.h>

#include <map>
using std::map;
using std::make_pair;


typedef map< string, shared_ptr<TRX_FLOW> > DB_FLOW;
static DB_FLOW db_flow;

static
TRX_FLOW & flow_get(const string &flow_name)
{
    DB_FLOW::iterator it = db_flow.find(flow_name);
    ENSURE(db_flow.end() != it,
            "flow \"%s\" not find", flow_name.c_str());

    return *(it->second.get());
}

static
void flow_create_impl(const string &flow_name)
{
    ENSURE(db_flow.end() == db_flow.find(flow_name),
            "flow \"%s\" already exist", flow_name.c_str());

    shared_ptr<TRX_FLOW> ptr(new TRX_FLOW());

    db_flow.insert(make_pair(flow_name,ptr));
}

static
void flow_destroy_impl(const string &flow_name)
{
    DB_FLOW::iterator it = db_flow.find(flow_name);
    ENSURE(db_flow.end() != it,
            "flow \"%s\" not find", flow_name.c_str());

    db_flow.erase(it);
}


static
void flow_set_tx_rate_impl(const string &flow_name, const RATE &rate, const BURST &burst)
{
    DB_FLOW::iterator it = db_flow.find(flow_name);
    ENSURE(db_flow.end() != it,
            "flow \"%s\" not find", flow_name.c_str());

    it->second->set_tx(rate, burst);
}

static
void flow_set_tx_port_impl(const string &flow_name, const PORT_NAME &port)
{
    DB_FLOW::iterator it = db_flow.find(flow_name);
    ENSURE(db_flow.end() != it,
            "flow \"%s\" not find", flow_name.c_str());

    it->second->set_tx(port);
}

static
void flow_set_tx_pkt_impl(const string &flow_name, const string &pkt_name, const string & track)
{
    DB_FLOW::iterator it = db_flow.find(flow_name);
    ENSURE(db_flow.end() != it,
            "flow \"%s\" not find", flow_name.c_str());

    FIELD_BLOCK &pkt = get_pkt(pkt_name);
    it->second->set_tx(pkt, track);
}

static
void flow_set_rx_port_impl(const string &flow_name, const PORT_NAME_SET &ports)
{
    DB_FLOW::iterator it = db_flow.find(flow_name);
    ENSURE(db_flow.end() != it,
            "flow \"%s\" not find", flow_name.c_str());

    it->second->set_rx(ports);
}

static
void flow_set_rx_offset_impl(const string &flow_name, UINT32 offset)
{
    DB_FLOW::iterator it = db_flow.find(flow_name);
    ENSURE(db_flow.end() != it,
            "flow \"%s\" not find", flow_name.c_str());

    it->second->set_rx(offset);
}

#endif


int CIRCLE_FLOW_LUA_C::trx_flow_create(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("unique_flow_create: %s\r\n", flow);

#ifndef ENV_UT
    flow_create_impl(flow);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::trx_flow_destroy(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("unique_flow_destroy: %s\r\n", flow);

#ifndef ENV_UT
    flow_destroy_impl(flow);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}


int CIRCLE_FLOW_LUA_C::trx_flow_set_tx_rate(void * lua_state)
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

    TRACE("unique_flow_set_tx_rate: %s, rate:%s, burst:%d\r\n", flow, string_any(rate).c_str(), burst.value);

#ifndef ENV_UT
    flow_set_tx_rate_impl(flow, rate, burst);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}


int CIRCLE_FLOW_LUA_C::trx_flow_set_tx_port(void * lua_state)
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

    TRACE("unique_flow_set_tx_port: %s, port:%s\r\n", flow, port.c_str());

#ifndef ENV_UT
    flow_set_tx_port_impl(flow, port);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::trx_flow_set_tx_pkt(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow, *pkt, *track;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    pkt = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    track = lua_tostring(L, index);

    TRACE("unique_flow_set_tx_pkt: %s, pkt:%s, track:%s\r\n", flow, pkt, track);

#ifndef ENV_UT
    flow_set_tx_pkt_impl(flow, pkt, track);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}


int CIRCLE_FLOW_LUA_C::trx_flow_set_rx_port(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;
    PORT_NAME_SET ports;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    {
        int nport;
        ENSURE(LUA_TTABLE == lua_type(L, ++index));
        nport = luaL_len(L, index);

        for(int i=1; i<=nport; i++) {
            ENSURE(LUA_TSTRING == lua_rawgeti(L, index, i));
            ports.insert(string(lua_tostring(L, -1)));
            lua_pop(L, 1);
        }
    }

    TRACE("unique_flow_set_rx_port: %s, %s \r\n",
            flow,
            string_any(ports).c_str());

#ifndef ENV_UT
    flow_set_rx_port_impl(flow, ports);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}


int CIRCLE_FLOW_LUA_C::trx_flow_set_rx_offset(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0, offset;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    offset = lua_tonumber(L, index);

    TRACE("unique_flow_set_rx_offset: %s, offset:%d\r\n", flow, offset);

#ifndef ENV_UT
    flow_set_rx_offset_impl(flow, (UINT32)offset);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}


int CIRCLE_FLOW_LUA_C::trx_flow_start(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("unique_flow_start: %s\r\n", flow);

#ifndef ENV_UT
    flow_get(flow).trx_start();
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::trx_flow_stop(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("unique_flow_stop: %s\r\n", flow);

#ifndef ENV_UT
    flow_get(flow).trx_stop();
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::trx_flow_snoop(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0, max_cnt;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    ENSURE(LUA_TNUMBER == lua_type(L, ++index));
    max_cnt = lua_tonumber(L, index);

    TRACE("unique_flow_snoop: %s, max=%d\r\n", flow, max_cnt);

#ifndef ENV_UT
    flow_get(flow).snoop(max_cnt);
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::trx_flow_dump(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("unique_flow_dump: %s\r\n", flow);

#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#else
    SNOOP_PKTS snoops;
    flow_get(flow).dump(snoops);

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

int CIRCLE_FLOW_LUA_C::trx_flow_latency_start(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("unique_flow_latency_start: %s\r\n", flow);

#ifndef ENV_UT
    flow_get(flow).lm_start();
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::trx_flow_latency_stop(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("unique_flow_latency_stop: %s\r\n", flow);

#ifndef ENV_UT
    flow_get(flow).lm_stop();
#endif

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::trx_flow_latency_show(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("unique_flow_latency_start: %s\r\n", flow);

#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#else
    int latency = flow_get(flow).lm_get();
    string str=" latency measured:";
    str += string_any(latency);
    str += " us ";
    lua_pushnumber(L, 1);
    lua_pushstring(L, str.c_str());
#endif

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
        str += left_padding("  name",    10);
        str += left_padding("  status",  10);
        str += left_padding("  latency", 10);
        str += left_padding("  tx_port", 10);
        str += left_padding("  tx_rate", 10);
        str += left_padding("  tx_burst",10);

        str += left_padding("  rx_offset",10);
        str += left_padding("  rx_ports",15);
        str += "\r\n=============================================================================================\r\n";

        run_once = false;
    }

    return str;
}

static
string flow_summary(const string &name, TRX_FLOW &flow)
{
    string str;
    TRX_FLOW::INFO info=flow.info();

    str += left_padding(name,       10);
    str += left_padding(info.is_trx_started ?
                            (info.is_snooping ? "snooping" : "started"):"stopped", 10);
    str += left_padding(info.is_lm_started ? "started" : "stopped", 10);
    str += left_padding(info.tx_port,  10);
    str += left_padding(info.tx_rate,  10);
    str += left_padding(info.tx_burst, 10);

    str += left_padding(info.rx_track_offset,10);
    str += left_padding(info.rx_ports, 15);

    return str;
}

static
string cnt_summary_title()
{
    string str;

    str += left_padding("name",  10);
    str += left_padding("tx.pkt",   12);
    str += left_padding("tx.byte",  12);
    str += left_padding("tx.pkt/s", 12);
    str += left_padding("tx.byte/s",12);
    str += left_padding("rx.pkt",   12);
    str += left_padding("rx.byte",  12);
    str += left_padding("rx.pkt/s", 12);
    str += left_padding("rx.byte/s",12);
    str += "\r\n==========================================================================================================\r\n";
    return str;

}

static
string cnt_summary(const string &name, TRX_FLOW &flow, bool is_clear)
{
    TRX_CNT cnt  = flow.counter(is_clear);
    TRX_CNT rate = flow.counter_rate();

    string str;

    str += left_padding(name,     10);
    str += left_padding(cnt.tx.pkt,   11) + " ";
    str += left_padding(cnt.tx.byte,  11) + " ";
    str += left_padding(rate.tx.pkt,  11) + " ";
    str += left_padding(rate.tx.byte, 11) + " ";
    str += left_padding(cnt.rx.pkt,   11) + " ";
    str += left_padding(cnt.rx.byte,  11) + " ";
    str += left_padding(rate.rx.pkt,  11) + " ";
    str += left_padding(rate.rx.byte, 11);
	str += "\r\n";

    return str;
}

static
string cnt_detail(const string &name, TRX_FLOW &flow, bool is_clear)
{
    TRX_CNT cnt  = flow.counter(is_clear);
    TRX_CNT rate = flow.counter_rate();

    string str;

    str += " ==== " + name + " ====\r\n";
    str += "\r\n tx.pkt   : " + string_any(cnt.tx.pkt);
    str += "\r\n tx.byte  : " + string_any(cnt.tx.byte);
    str += "\r\n tx.pkt/s : " + string_any(rate.tx.pkt);
    str += "\r\n tx.byte/s: " + string_any(rate.tx.byte);
    str += "\r\n rx.pkt   : " + string_any(cnt.rx.pkt);
    str += "\r\n rx.byte  : " + string_any(cnt.rx.byte);
    str += "\r\n rx.pkt/s : " + string_any(rate.rx.pkt);
    str += "\r\n rx.byte/s: " + string_any(rate.rx.byte);

    return str;
}

#endif

int CIRCLE_FLOW_LUA_C::trx_flow_show(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    TRACE("unique_flow_show: %s\r\n", flow);

#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#else
    string str = flow_summary_title();
    if(0==strlen(flow)) {
        for(DB_FLOW::iterator it=db_flow.begin(); it!=db_flow.end(); it++) {
            str += flow_summary(it->first, *(it->second.get()));
        }
    } else {
        str += flow_summary(flow, flow_get(flow));
    }

    lua_pushnumber(L, 1);
    lua_pushstring(L, str.c_str());
#endif

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::trx_flow_cnt(void * lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *flow;
    int index=0, is_clear;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    flow = lua_tostring(L, index);

    ENSURE(LUA_TBOOLEAN == lua_type(L, ++index));
    is_clear = lua_toboolean(L, index);

    TRACE("unique_flow_cnt: %s, clr: %d\r\n", flow, is_clear);

#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#else
    string str;
    if(0==strlen(flow)) {
        str += cnt_summary_title();
        for(DB_FLOW::iterator it=db_flow.begin(); it!=db_flow.end(); it++) {
            str += cnt_summary(it->first, *(it->second.get()), is_clear);
        }
    } else {
        str += cnt_detail(flow, flow_get(flow), is_clear);
    }

    lua_pushnumber(L, 1);
    lua_pushstring(L, str.c_str());
#endif

    EXP_FREE_END_LUA_C;

    return 2;
}
