
#include <algorithm>
using std::for_each;
#include <stdio.h>
#include <sstream>
using std::ostringstream;
using std::endl;

#include <utility/export/error.h>
#include "utility/export/base_type.h"
#include "utility/export/smart_ptr_u.h"
#include <utility/export/cmd.h>
using namespace UTILITY;

#include "circle_flow/export/flow.h"
#include "circle_flow/export/pkt_lib.h"
#include "circle_flow/export/qual.h"
using namespace CIRCLE_FLOW;

/* cli cmd to verify the circle_flow basic functionality
 * steps:
 *   1, loopback the first front port, manually by bcm sdk cmd "port xxx loopback=mac"
 *   2, run cmd "minit basic ..." to verify TX_FLOW/RX_FLOW, and "minit trx ..." for TRX_FLOW
 *   3, restore port from loopback mode, manually by bcm sdk cmd "port xxx loopback=none"
 * */


static
BYTES increament_bytes(UINT16 length)
{
    static BYTES pattern;
    const UINT16 MAX_SIZE = 256;

    if(0 == pattern.size()) {
        for(UINT16 i=0; i<MAX_SIZE; i++) {
            pattern.push_back((UINT8)i);
        }
    }

    BYTES bytes;

    while(length > MAX_SIZE) {
        bytes.insert(bytes.end(), pattern.begin(), pattern.end());
        length -= MAX_SIZE;
    }

    if(length>0) {
        bytes.insert(bytes.end(), pattern.begin(), pattern.begin()+length);
    }

    return bytes;
}

static
PORT_NAME test_port()
{
    static PORT_NAME port = *(PORT::get_port_all().begin());
    return port;
}

///////////////basic/////////////////

static shared_ptr<TX_FLOW> tx_flow;
static shared_ptr<RX_FLOW> rx_flow;

static void basic_create(void)
{
    //tx
    shared_ptr<TX_FLOW> tx(new TX_FLOW());

    tx->set(test_port());
    tx->set(RATE(RATE::MBITS_PER_SECOND, 10), BURST());

    using namespace FB_ETH_II;

    FIELD_BLOCK pkt(make_ETH_II());

    ENSURE(pkt.size_of_bit()>0);

    pkt.field(DST_MAC) = "00-00-00-00-00-01";
    pkt.field(SRC_MAC) = "00-00-00-00-00-02";
    pkt.field(ETH_TYPE) = "86-dd";

    using namespace FB_IPV6;
    pkt.extend(L3_PAYLOAD, make_IPV6());

    pkt.block(L3_PAYLOAD).field(PLEN) = BYTES();
    pkt.block(L3_PAYLOAD).field(L4_PAYLOAD) = BYTES(20, 0x33);
    pkt.block(L3_PAYLOAD).field(SRC_IP) = "00-01-02-03-04-05-06-07-08-09-0a-0b-0c-0d-0e-0f";
    pkt.block(L3_PAYLOAD).field(DST_IP) = "00-10-20-30-40-50-60-70-80-90-a0-b0-c0-d0-e0-f0";

    tx->set(PACKETS(1,(BYTES)pkt));

    //rx
    shared_ptr<RX_FLOW> rx(new RX_FLOW());

    QUAL qual;
    qual.in_ports.insert(test_port());
    qual.pdfs.insert(QUAL_HELPER::make_pdf_rule(QUAL::dst_mac, (BYTES)pkt.field(DST_MAC)));
    qual.pdfs.insert(QUAL_HELPER::make_pdf_rule(QUAL::src_mac, (BYTES)pkt.field(SRC_MAC)));
    qual.pdfs.insert(QUAL_HELPER::make_pdf_rule(QUAL::dst_ip6, (BYTES)pkt.block(L3_PAYLOAD).field(DST_IP)));
    qual.pdfs.insert(QUAL_HELPER::make_pdf_rule(QUAL::src_ip6, (BYTES)pkt.block(L3_PAYLOAD).field(SRC_IP)));

    rx->set(qual);

    tx_flow=tx;
    rx_flow=rx;
}

static void basic_start(void)
{
    if(rx_flow.get()) {
        rx_flow->start();
    }

    if(tx_flow.get()) {
        tx_flow->start();
    }
}

static void basic_stop(void)
{
    if(tx_flow.get()) {
        tx_flow->stop();
    }

    if(rx_flow.get()) {
        rx_flow->stop();
    }
}

static void print_counter(const TRX_CNT &cnt)
{
    CMD::printf("\r\n tx.pkt: %llu, tx.byte: %llu, rx.pkt: %llu, rx.byte: %llu",
                cnt.tx.pkt, cnt.tx.byte, cnt.rx.pkt, cnt.rx.byte);
}

static void basic_counter(void)
{
    if(tx_flow.get() && rx_flow.get()) {

        TRX_CNT cnt;
        cnt.tx = tx_flow->counter();
        cnt.rx = rx_flow->counter();

        CMD::printf("\r\n ===cnt===");
        print_counter(cnt);

        cnt.tx = tx_flow->counter_rate();
        cnt.rx = rx_flow->counter_rate();

        CMD::printf("\r\n ===rate===");
        print_counter(cnt);
    }
}

static void basic_rate(UINT32 _rate, UINT32 _burst)
{

    if(tx_flow.get()) {

        RATE rate;
        rate.type = RATE::MBITS_PER_SECOND;
        rate.value = _rate;

        BURST burst;
        burst.type = BURST::PKT;
        burst.value = _burst;

        tx_flow->set(rate, burst);
    }
}

static
void snoop_dump(const SNOOP_PKT &snoop)
{
    ostringstream oss;
    oss<<endl<<"time stamp:"<<snoop.time_stamp<<endl;

    oss.setf (std::ios::hex , std::ios::basefield);
    oss.setf (std::ios::right , std::ios::adjustfield);
    oss.unsetf (std::ios::showbase);

    UINT8 dump_len=32;
    dump_len = snoop.pkt.size()<dump_len ? snoop.pkt.size() : dump_len;

    UINT8 i=0;
    for(; i<(dump_len-1); i++) {

        oss.width(2);
        oss.fill('0');

        oss<<(short)snoop.pkt[i];

        if(15 == (i%16)) {
            oss<<endl;
        } else {
            oss<<":";
        }
    }

    oss.width(2);
    oss.fill('0');

    oss<<(unsigned short)snoop.pkt[i];

    CMD::printf(oss.str().c_str());
}

static void basic_snoop(UINT32 num_of_pkt)
{
    if(rx_flow.get()) {

        rx_flow->snoop(num_of_pkt);

        if(0 == num_of_pkt) {
            SNOOP_PKTS pkts;
            rx_flow->dump(pkts);

            for_each(pkts.begin(), pkts.end(), snoop_dump);
        }
    }
}

static void basic_reset(void)
{
    tx_flow.reset();

    rx_flow.reset();
}

///////////////trx//////////////////
static shared_ptr<TRX_FLOW> trx_flow;

static void trx_create(void)
{
    shared_ptr<TRX_FLOW> trx(new TRX_FLOW());

    trx->set_tx(test_port());
    trx->set_tx(RATE(RATE::MBITS_PER_SECOND, 10), BURST());

    using namespace FB_ETH_II;

    FIELD_BLOCK pkt(make_ETH_II());

    ENSURE(pkt.size_of_bit()>0);

    pkt.field(DST_MAC) = "00-00-00-00-00-01";
    pkt.field(SRC_MAC) = "00-00-00-00-00-02";
    pkt.field(ETH_TYPE) = "08-00";
    pkt.field(L3_PAYLOAD) = increament_bytes(64);

    using namespace FB_FLOW_TRACK_TAG;
    pkt.insert(FCS, make_FLOW_TRACK_TAG());

    trx->set_tx(pkt, FLOW_TRACK_TAG);

    PORT_NAME_SET ports;
    ports.insert(test_port());
    trx->set_rx(ports);

    UINT32 offset_of_rx_flow_track = pkt.offset_of_bit(FLOW_TRACK_TAG);
    ENSURE(0 == (offset_of_rx_flow_track%8));
    trx->set_rx(offset_of_rx_flow_track/8);

    trx_flow=trx;
}

static void trx_start()
{
    if(trx_flow.get()) {
        trx_flow->trx_start();
    }
}

static void trx_stop()
{
    if(trx_flow.get()) {
        trx_flow->trx_stop();
    }
}

static void trx_counter(void)
{
    if(trx_flow.get()) {
        CMD::printf("\r\n ===cnt===");
        print_counter(trx_flow->counter());
    }

    if(trx_flow.get()) {
        CMD::printf("\r\n ===rate===");
        print_counter(trx_flow->counter_rate());
    }
}

static void trx_rate(UINT32 _rate, UINT32 _burst)
{

    if(trx_flow.get()) {

        RATE rate;
        rate.type = RATE::MBITS_PER_SECOND;
        rate.value = _rate;

        BURST burst;
        burst.type = BURST::PKT;
        burst.value = _burst;

        trx_flow->set_tx(rate, burst);
    }
}

static void trx_snoop(UINT32 num_of_pkt)
{
    if(trx_flow.get()) {

        trx_flow->snoop(num_of_pkt);

        if(0 == num_of_pkt){

            SNOOP_PKTS pkts;
            trx_flow->dump(pkts);

            for_each(pkts.begin(), pkts.end(), snoop_dump);
        }
    }
}

static void trx_reset(void)
{
    trx_flow.reset();
}


static
void helper(void)
{
    static char _help [] =
        " basic [create/reset | start/stop | counter | rate | snoop]\r\n"
        " trx   [create/reset | start/stop | counter | rate | snoop | latency]\r\n";;

    CMD::printf("%s", _help);

}

static
void handler(const CMD::INPUTS &inputs)
{
    size_t size=inputs.size(), index=0;
    ENSURE(size>index);

    if( "basic" == inputs[index] ) {

        ENSURE(size>(++index));

        if( "create" == inputs[index] ) {

            basic_create();

        } else if( "start" == inputs[index] ){

            basic_start();

        } else if( "stop" == inputs[index] ){

            basic_stop();

        } else if( "counter" == inputs[index] ){

            basic_counter();

        } else if( "rate" == inputs[index] ){

            ENSURE(size > (index+2));

            UINT32 rate=0, burst=0;

            ENSURE(1== sscanf( inputs[++index].c_str(), "%d", &rate ));
            ENSURE(1== sscanf( inputs[++index].c_str(), "%d", &burst));

            basic_rate(rate, burst);

        } else if( "snoop" ==  inputs[index] ){

            ENSURE(size > (++index));

            UINT32 num_of_pkt=0;

            ENSURE(1== sscanf( inputs[index].c_str(), "%d", &num_of_pkt ));

            basic_snoop(num_of_pkt);

        } else if("reset" == inputs[index] ){

            basic_reset();
        }

    } else if( "trx" == inputs[index]) {

        ENSURE(size>(++index));

        static bool latency_started = false;

        if( "create" == inputs[index] ) {

            trx_create();

        } else if( "start" == inputs[index] ){

            trx_start();

        } else if( "stop" == inputs[index] ){

            trx_stop();

            latency_started = false;

        } else if( "counter" == inputs[index]){

            trx_counter();

        } else if( "rate" == inputs[index] ){

            ENSURE(size > (index+2));
            UINT32 rate=0, burst=0;
            ENSURE(1== sscanf( inputs[++index].c_str(), "%d", &rate ));
            ENSURE(1== sscanf( inputs[++index].c_str(), "%d", &burst));

            trx_rate(rate, burst);

        } else if( "snoop" == inputs[index] ){

            ENSURE(size > (++index));
            UINT32 num_of_pkt=0;
            ENSURE(1== sscanf( inputs[index].c_str(), "%d", &num_of_pkt ));

            trx_snoop( num_of_pkt);

        } else if( "latency" == inputs[index] ){

            if(false == latency_started) {
                trx_flow->lm_start();
                latency_started = true;
            } else {
                CMD::printf("\r\n latency measured : %d us", trx_flow->lm_get());

                trx_flow->lm_stop();
                latency_started = false;
            }

        } else if( "reset" == inputs[index] ){
            trx_reset();
        }
    }
}

static CMD::AUTO_LINK __cmd("minit", handler, helper);

