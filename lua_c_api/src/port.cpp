#include "utility/export/error.h"
using namespace UTILITY;

#include "circle_flow.h"
#include "helper.h"

#include "circle_flow/export/type.h"
#include "circle_flow/export/port.h"
using namespace CIRCLE_FLOW;
using namespace CIRCLE_FLOW::PORT;
extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
}

#include <algorithm>
using std::for_each;

#ifndef ENV_UT

static
string status_summary_title()
{
    static string str;
    static bool run_once = true;

    if(run_once) {
        str += left_padding(" name",   10);
        str += left_padding(" itf",    5);
        str += left_padding(" link",   5);
        str += left_padding(" AN",  5);
        str += left_padding(" DS",  12);
        str += "\r\n=========================================\r\n";

        run_once = false;
    }

    return str;
}

static
string status_summary_port(const PORT_NAME &port)
{
    string str;

    str += left_padding(port, 10);
    str += left_padding(get_itf(port), 5);

    STATUS status = get_status(port);
    str += left_padding(status.link ? "up" : "down", 5);
    str += left_padding(status.an_enable ? "Y" : "N", 5);
    str += left_padding(status.ds,   12);
    str += "\r\n";

    return str;
}

static
string status_summary(const PORT_NAME_SET &ports)
{
    string str = status_summary_title();

    for(PORT_NAME_SET::const_iterator it = ports.begin(); it!=ports.end(); it++){
        str += status_summary_port(*it);
    }

    return str;
}

#endif

int CIRCLE_FLOW_LUA_C::port_status(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *port;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    port = lua_tostring(L, index);

    TRACE("port_status:%s\r\n", port);

#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#else
    PORT_NAME_SET ports;

    if(0 == strlen(port)) {
        ports = get_port_all();
    } else {
        ports.insert(ports.end(), port);
    }

    lua_pushnumber(L, 1);
    lua_pushstring(L, status_summary(ports).c_str());
#endif

    EXP_FREE_END_LUA_C;

    return 2;
}


#ifndef ENV_UT

static
string counter_summary_title()
{
    static string str;
    static bool run_once = true;

    if(run_once) {
        str += left_padding("  name",     10);
        str += left_padding("  tx.pkt",   12);
        str += left_padding("  tx.byte",  12);
        str += left_padding("  rx.pkt",   12);
        str += left_padding("  rx.byte",  12);
        str += left_padding("  tx.pkt/s", 12);
        str += left_padding("  tx.byte/s",12);
        str += left_padding("  rx.pkt/s", 12);
        str += left_padding("  rx.byte/s",12);
        str += "\r\n=============================================================================================================\r\n";

        run_once = false;
    }

    return str;
}

static
string counter_summary_port(const PORT_NAME &port, bool is_clear)
{
    string str;

    TRX_CNT cnt = get_cnt(port, is_clear);
    TRX_CNT rate = get_rate(port);

    str += left_padding(port, 10);
    str += left_padding(cnt.tx.pkt,  12);
    str += left_padding(cnt.tx.byte, 12);
    str += left_padding(cnt.rx.pkt,  12);
    str += left_padding(cnt.rx.byte, 12);
    str += left_padding(rate.tx.pkt,  12);
    str += left_padding(rate.tx.byte, 12);
    str += left_padding(rate.rx.pkt,  12);
    str += left_padding(rate.rx.byte, 12);
    str += "\r\n";

    return str;
}

static
string counter_summary(const PORT_NAME_SET &ports, bool is_clear)
{
    string str = counter_summary_title();

    for(PORT_NAME_SET::const_iterator it = ports.begin(); it!=ports.end(); it++){
        str += counter_summary_port(*it, is_clear);
    }

    return str;
}

#endif

int CIRCLE_FLOW_LUA_C::port_cnt(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *port;
    int index=0, is_clear;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    port = lua_tostring(L, index);

    ENSURE(LUA_TBOOLEAN == lua_type(L, ++index));
    is_clear = lua_toboolean(L, index);

    TRACE("port_cnt:%s, clr:%d\r\n", port, is_clear);

#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#else
    PORT_NAME_SET ports;

    if (0 == strlen(port)) {
        ports = get_port_all();
    } else {
        ports.insert(ports.end(), port);
    }

    lua_pushnumber(L, 1);
    lua_pushstring(L, counter_summary(ports, is_clear).c_str());
#endif

    EXP_FREE_END_LUA_C;

    return 2;
}

#ifndef ENV_UT

static
struct ITF_MAP {
    const char * name;
    INTERFACE itf;
} itf_map [] = {
        {"SFP", ITF_SFP},
        {"RJ45",ITF_RJ45}
};

static
INTERFACE str_to_interface(const string &str)
{
    for(UINT8 i=0; i<ARRAY_NUM(itf_map); i++) {
        if(itf_map[i].name == str) {
            return itf_map[i].itf;
        }
    }

    ERROR("invalid interface name:%s", str.c_str());
    return ITF_END; //only for compile warning
}

static
ostream & operator << (ostream & os, const INTERFACE &itf)
{
    for(UINT8 i=0; i<ARRAY_NUM(itf_map); i++) {
        if(itf_map[i].itf == itf) {
            os<<itf_map[i].name;
            return os;
        }
    }

    ERROR("invalid interface value:%d", itf);
    return os; //only for compile warning
}

static
struct DS_MAP {
    const char * name;
    DUPLEX_SPEED ds;
} ds_map [] = {
        {"FD_10MB",   FD_10MB},
        {"HD_10MB",   HD_10MB},
        {"FD_100MB",  FD_100MB},
        {"HD_100MB",  HD_100MB},
        {"FD_1000MB", FD_1000MB},
        {"HD_1000MB", HD_1000MB},
        {"FD_2500MB", FD_2500MB},
        {"HD_2500MB", HD_2500MB},
        {"FD_10GB",   FD_10GB},
        {"HD_10GB",   HD_10GB},
        {"NA",   DS_END}
};

static
DUPLEX_SPEED str_to_ds( const string &str)
{
    for(UINT8 i=0; i<ARRAY_NUM(ds_map); i++) {
        if(ds_map[i].name == str) {
            return ds_map[i].ds;
        }
    }

    ERROR("invalid DUPLEX_SPEED name:%s", str.c_str());
    return DS_END; //only for compile warning
}

static
ostream & operator << (ostream & os, const DUPLEX_SPEED &ds)
{
    for(UINT8 i=0; i<ARRAY_NUM(ds_map); i++) {
        if(ds_map[i].ds == ds) {
            os<<ds_map[i].name;
            return os;
        }
    }

    ERROR("invalid DUPLEX_SPEED value:%d", ds);
    return os; //only for compile warning
}

#endif

int CIRCLE_FLOW_LUA_C::port_set_itf(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *port, *itf;
    int index=0;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    port = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    itf = lua_tostring(L, index);

    TRACE("port_set_itf:%s, %s\r\n", port, itf);

#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#else
    set_itf(port, str_to_interface(itf));

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#endif

    EXP_FREE_END_LUA_C;

    return 2;
}

int CIRCLE_FLOW_LUA_C::port_set_forced(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *port, *ds;
    int index=0, pause;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    port = lua_tostring(L, index);

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    ds = lua_tostring(L, index);

    ENSURE(LUA_TBOOLEAN == lua_type(L, ++index));
    pause = lua_toboolean(L, index);

    TRACE("port_set_force:%s, %s, pause:%d\r\n", port, ds, pause);

#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#else

    MODE mode;
    mode.ds = str_to_ds(ds);
    mode.pause = pause;
    set_forced(port, mode);

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#endif

    EXP_FREE_END_LUA_C;

    return 2;
}


int CIRCLE_FLOW_LUA_C::port_set_auto(void *lua_state)
{
    lua_State *L = static_cast<lua_State *> (lua_state);

    EXP_FREE_START_LUA_C;

    const char *port;
    int index=0, pause, nds;
    vector<string> ds;

    ENSURE(LUA_TSTRING == lua_type(L, ++index));
    port = lua_tostring(L, index);

    ENSURE(LUA_TTABLE == lua_type(L, ++index));
    nds = luaL_len(L, index);

    for(int i=1; i<=nds; i++) {
        ENSURE(LUA_TSTRING == lua_rawgeti(L, index, i));
        ds.push_back(string(lua_tostring(L, -1)));
        lua_pop(L, 1);
    }

    ENSURE(LUA_TBOOLEAN == lua_type(L, ++index));
    pause = lua_toboolean(L, index);

    string str_ds;
    for(vector<string>::iterator it=ds.begin(); it!=ds.end(); it++) {
        str_ds += *it + " ";
    }
    TRACE("port_set_auto:%s, %s, pause:%d\r\n", port, str_ds.c_str(), pause);

#ifdef ENV_UT
    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#else

    ADVERT advert;
    DS_SET ds_set;
    for(vector<string>::iterator it=ds.begin(); it!=ds.end(); it++) {
        ds_set.insert(str_to_ds(*it));
    }
    advert.ds_set = ds_set;
    advert.pause = pause;
    set_auto_nego(port, advert);

    lua_pushnumber(L, 0);
    lua_pushstring(L, "OK");
#endif

    EXP_FREE_END_LUA_C;

    return 2;
}

