
#ifndef EXPORT_CIRCLE_FLOW_LUA_C_H_
#define EXPORT_CIRCLE_FLOW_LUA_C_H_

namespace CIRCLE_FLOW_LUA_C {

    int pkt_create_eth(void *lua_state);
    int pkt_create_raw(void *lua_state);
    int pkt_destroy(void *lua_state);
    int pkt_copy(void *lua_state);

    int pkt_set_value(void *lua_state);
    int pkt_set_step(void *lua_state);
    int pkt_set_random(void *lua_state);
    int pkt_set_pattern(void *lua_state);
    int pkt_show(void *lua_state);

    int pkt_get_offset_bit(void *lua_state);

    int pkt_field_append(void *lua_state);
    int pkt_field_insert(void *lua_state);
    int pkt_field_remove(void *lua_state);
    int pkt_field_extend(void *lua_state);
    int pkt_field_resize(void *lua_state);

    int tx_flow_create(void *lua_state);
    int tx_flow_destroy(void *lua_state);
    int tx_flow_set_rate(void * lua_state);
    int tx_flow_set_port(void * lua_state);
    int tx_flow_set_pkt(void * lua_state);
    int tx_flow_start(void * lua_state);
    int tx_flow_stop(void * lua_state);
    int tx_flow_show(void * lua_state);
    int tx_flow_cnt(void * lua_state);

    int rx_flow_create(void *lua_state);
    int rx_flow_destroy(void *lua_state);
    int rx_flow_set_port(void * lua_state);
    int rx_flow_set_udf(void * lua_state);
    int rx_flow_set_udf_prefix(void * lua_state);
    int rx_flow_set_pdf(void * lua_state);
    int rx_flow_set_pdf_prefix(void * lua_state);
    int rx_flow_start(void * lua_state);
    int rx_flow_stop(void * lua_state);
    int rx_flow_snoop(void * lua_state);
    int rx_flow_dump(void * lua_state);
    int rx_flow_show(void * lua_state);
    int rx_flow_cnt(void * lua_state);

    int trx_flow_create(void *lua_state);
    int trx_flow_destroy(void *lua_state);
    int trx_flow_set_tx_port(void * lua_state);
    int trx_flow_set_tx_rate(void * lua_state);
    int trx_flow_set_tx_pkt(void * lua_state);
    int trx_flow_set_rx_port(void * lua_state);
    int trx_flow_set_rx_offset(void * lua_state);
    int trx_flow_start(void * lua_state);
    int trx_flow_stop(void * lua_state);
    int trx_flow_snoop(void * lua_state);
    int trx_flow_dump(void * lua_state);
    int trx_flow_latency_start(void * lua_state);
    int trx_flow_latency_stop(void * lua_state);
    int trx_flow_latency_show(void * lua_state);
    int trx_flow_show(void * lua_state);
    int trx_flow_cnt(void * lua_state);

    int port_status(void *lua_state);
    int port_set_itf(void *lua_state);
    int port_set_forced(void *lua_state);
    int port_set_auto(void *lua_state);
    int port_cnt(void *lua_state);

    void register_lib(void *lua_state);

}



#endif
