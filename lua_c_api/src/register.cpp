
#include "circle_flow.h"
using namespace CIRCLE_FLOW_LUA_C;

extern "C" {
#include "lua/lua.h"
}

#define REGISTER(func,name) \
        lua_pushcfunction(L, (lua_CFunction)func);\
        lua_setglobal(L, name);

void CIRCLE_FLOW_LUA_C::register_lib(void *lua_state)
{
    lua_State *L = (lua_State *) lua_state;

    REGISTER(pkt_create_eth, "lua_c_pkt_create_eth");
    REGISTER(pkt_create_raw, "lua_c_pkt_create_raw");
    REGISTER(pkt_copy,       "lua_c_pkt_copy");
    REGISTER(pkt_destroy,    "lua_c_pkt_destroy");

    REGISTER(pkt_set_value,  "lua_c_pkt_set_value");
    REGISTER(pkt_set_step,   "lua_c_pkt_set_step");
    REGISTER(pkt_set_random, "lua_c_pkt_set_random");
    REGISTER(pkt_set_pattern,"lua_c_pkt_set_pattern");
    REGISTER(pkt_show,       "lua_c_pkt_show");
    REGISTER(pkt_get_offset_bit, "lua_c_pkt_get_offset_bit");

    REGISTER(pkt_field_append,   "lua_c_pkt_field_append");
    REGISTER(pkt_field_insert,   "lua_c_pkt_field_insert");
    REGISTER(pkt_field_remove,   "lua_c_pkt_field_remove");
    REGISTER(pkt_field_extend,   "lua_c_pkt_field_extend");
    REGISTER(pkt_field_resize,   "lua_c_pkt_field_resize");

    REGISTER(tx_flow_create,    "lua_c_tx_flow_create");
    REGISTER(tx_flow_destroy,   "lua_c_tx_flow_destroy");
    REGISTER(tx_flow_set_rate,  "lua_c_tx_flow_set_rate");
    REGISTER(tx_flow_set_port,  "lua_c_tx_flow_set_port");
    REGISTER(tx_flow_set_pkt,   "lua_c_tx_flow_set_pkt");
    REGISTER(tx_flow_start,     "lua_c_tx_flow_start");
    REGISTER(tx_flow_stop,      "lua_c_tx_flow_stop");
    REGISTER(tx_flow_show,      "lua_c_tx_flow_show");
    REGISTER(tx_flow_cnt,       "lua_c_tx_flow_cnt");

    REGISTER(rx_flow_create,    "lua_c_rx_flow_create");
    REGISTER(rx_flow_destroy,   "lua_c_rx_flow_destroy");
    REGISTER(rx_flow_set_port,  "lua_c_rx_flow_set_port");
    REGISTER(rx_flow_set_udf,   "lua_c_rx_flow_set_udf");
    REGISTER(rx_flow_set_pdf,   "lua_c_rx_flow_set_pdf");
    REGISTER(rx_flow_set_udf_prefix,   "lua_c_rx_flow_set_udf_prefix");
    REGISTER(rx_flow_set_pdf_prefix,   "lua_c_rx_flow_set_pdf_prefix");
    REGISTER(rx_flow_start,     "lua_c_rx_flow_start");
    REGISTER(rx_flow_stop,      "lua_c_rx_flow_stop");
    REGISTER(rx_flow_snoop,     "lua_c_rx_flow_snoop");
    REGISTER(rx_flow_dump,      "lua_c_rx_flow_dump");
    REGISTER(rx_flow_show,      "lua_c_rx_flow_show");
    REGISTER(rx_flow_cnt,       "lua_c_rx_flow_cnt");

    REGISTER(trx_flow_create,        "lua_c_trx_flow_create");
    REGISTER(trx_flow_destroy,       "lua_c_trx_flow_destroy");
    REGISTER(trx_flow_set_tx_port,   "lua_c_trx_flow_set_tx_port");
    REGISTER(trx_flow_set_tx_rate,   "lua_c_trx_flow_set_tx_rate");
    REGISTER(trx_flow_set_tx_pkt,    "lua_c_trx_flow_set_tx_pkt");
    REGISTER(trx_flow_set_rx_port,   "lua_c_trx_flow_set_rx_port");
    REGISTER(trx_flow_set_rx_offset, "lua_c_trx_flow_set_rx_offset");
    REGISTER(trx_flow_start,         "lua_c_trx_flow_start");
    REGISTER(trx_flow_stop,          "lua_c_trx_flow_stop");
    REGISTER(trx_flow_snoop,         "lua_c_trx_flow_snoop");
    REGISTER(trx_flow_dump,          "lua_c_trx_flow_dump");
    REGISTER(trx_flow_latency_start, "lua_c_trx_flow_latency_start");
    REGISTER(trx_flow_latency_stop,  "lua_c_trx_flow_latency_stop");
    REGISTER(trx_flow_latency_show,  "lua_c_trx_flow_latency_show");
    REGISTER(trx_flow_show,          "lua_c_trx_flow_show");
    REGISTER(trx_flow_cnt,           "lua_c_trx_flow_cnt");

    REGISTER(port_status,       "lua_c_port_status");
    REGISTER(port_set_itf,      "lua_c_port_set_itf");
    REGISTER(port_set_forced,   "lua_c_port_set_forced");
    REGISTER(port_set_auto,     "lua_c_port_set_auto");
    REGISTER(port_cnt,          "lua_c_port_cnt");
}

