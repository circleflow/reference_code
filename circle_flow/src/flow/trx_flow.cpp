
#include "defs.h"
#include "port_impl.h"
#include "flow.h"
#include "tx.h"
#include "rx.h"
#include "flow_track.h"
#include "latency_cac.h"
using namespace QUAL_HELPER;

#include "utility/export/smart_ptr_u.h"


static
QUAL::UDF_RULE make_udf_rule(UINT32 offset_of_byte,
                             const QUAL::MATCH &match)
{
    QUAL::UDF_RULE rule;

    rule.first.offset = offset_of_byte;
    rule.first.size = FLOW_TRACK::size();

    rule.second = match;

    ENSURE(rule.first.size == rule.second.mask.size());

    return rule;
}

struct TRX_FLOW::IMPL {

    PORT_NAME      tx_port;
    FIELD_BLOCK    tx_pkt;
    FIELD::INDEX   tx_flow_track;
    UINT32 tx_track_offset;

    RATE  tx_rate;
    BURST tx_burst;

    PORT_NAME_SET  rx_ports;
    UINT32 rx_track_offset;

    shared_ptr<FLOW_TRACK> flow_track;

    shared_ptr<PORT_IMPL::PAIR_CB> pair_notify;
    shared_ptr<PORT_IMPL::LINK_CB> link_notify;

    shared_ptr<TX> tx;
    shared_ptr<RX> rx;

    shared_ptr<TX_LATENCY> tx_latency;
    shared_ptr<RX_LATENCY> rx_latency;

    shared_ptr<LATENCY_CAC> latency_cac;

    TRX_CNT cnt_base;

    bool is_trx_started;
    bool is_lm_started;

    bool is_snooping;
    UINT16 max_snoop_pkt;
    SNOOP_PKTS snoop_pkts;

    TIMER_SAFE cnt_rate_cacer;

    TRX_CNT cnt_rate_prev;
    TRX_CNT cnt_rate;

    IMPL()
    : tx_rate(RATE::KBITS_PER_SECOND, 1000),
      tx_burst(BURST::PKT, 0),
      is_trx_started(false),
      is_lm_started(false),
      is_snooping(false)
    {
        cnt_rate_cacer.set_op(bind(&IMPL::cnt_rate_cac, this));
    }

    void trx_start()
    {
        //in case of failure, resource would be free
        trx_stop();

        shared_ptr<FLOW_TRACK> ptr_track(new FLOW_TRACK());

        //tx valid check
        ENSURE(!tx_port.empty(),
                "tx port unspecified");
        ENSURE(true == port_link(PORT_IMPL::get_front(tx_port)),
                "port link down");
        ENSURE(tx_pkt.field(tx_flow_track).size_of_bit() == (ptr_track->size()*8),
                "flow track invalid");

        //rx valid check
        ENSURE(rx_ports.size()>0,
                "rx port unspecified");

        //rx qual
        QUAL qual;
        qual.in_ports = rx_ports;
        qual.hit_pri = QUAL::MAX_HIT_PRI + 1;  //ensure priority than RX_FLOW
        qual.udfs.insert(make_udf_rule(rx_track_offset, ptr_track->flow_match()));

        //allocate resource, use local ptr, in case of failure, resource would be automatically free
        shared_ptr<RX> ptr_rx(new RX(qual));

        //start traffic
        tx_pkt.field(tx_flow_track)=ptr_track->flow_bytes();

        shared_ptr<TX> ptr_tx(new TX(tx_port));
        ptr_tx->counter(true);
        ptr_tx->start(PACKETS(tx_pkt), tx_rate, tx_burst);

        //register port notify
        shared_ptr<PORT_IMPL::PAIR_CB> ptr_pair(new PORT_IMPL::PAIR_CB);
        *ptr_pair = bind(&IMPL::on_pair_change, this, placeholders::_1, placeholders::_2, placeholders::_3);
        pair_notify_bind(ptr_pair, make_ports(rx_ports, tx_port), false);

        shared_ptr<PORT_IMPL::LINK_CB> ptr_link(new PORT_IMPL::LINK_CB);
        *ptr_link = bind(&IMPL::on_link_change, this, placeholders::_1, placeholders::_2);
        link_notify_bind(ptr_link, make_ports(tx_port), false);

        cnt_base = TRX_CNT();
        cnt_rate = TRX_CNT();
        cnt_rate_prev = TRX_CNT();
        cnt_rate_cacer.start(RATE_CAC_INTERVAL, true);

        flow_track = ptr_track;
        tx = ptr_tx;
        rx = ptr_rx;
        pair_notify = ptr_pair;
        link_notify = ptr_link;

        is_trx_started = true;
    }

    void trx_stop()
    {
        if(is_trx_started) {

            link_notify.reset();
            pair_notify.reset();

            cnt_rate_cacer.stop();
            cnt_rate = TRX_CNT();

            lm_stop();
            snoop_stop();

            tx->stop();
            cnt_base = counter(true);

            tx.reset();
            rx.reset();

            flow_track.reset();

            is_trx_started = false;
        }
    }

    void on_link_change(const PORT_NAME &port, bool link)
    {
        if(false==link) trx_stop();
    }

    void on_pair_change(const PORT_NAME &, const PORT_ID &, const PORT_ID &)
    {
        trx_stop();
    }

    void on_lm_rx (UINT8 unit, const PACKET &pkt, UINT32 time_stamp)
    {
        ENSURE(pkt.size()>(rx_track_offset+flow_track->size()));

        PACKET::const_iterator it = pkt.begin() + rx_track_offset;
        BYTES bytes(it, it+flow_track->size());

        latency_cac->set_rx_timestamp(flow_track->extract_sn(bytes), time_stamp);

        if(is_snooping && snoop_pkts.size()<max_snoop_pkt) {
            SNOOP_PKT snoop_pkt;
            snoop_pkt.time_stamp  = time_stamp;
            snoop_pkt.pkt = pkt;

            snoop_pkts.push_back(snoop_pkt);
        }
    }

    void on_lm_tx(UINT8 unit, const PACKET &pkt, UINT32 time_stamp)
    {
        ENSURE(pkt.size()>(tx_track_offset+flow_track->size()));

        PACKET::const_iterator it = pkt.begin() + tx_track_offset;
        BYTES bytes(it, it+flow_track->size());

        TRC_LATENCY("\r\n pkt dump in tx observer:");
        TRC_LATENCY(bytes_dump(pkt, pkt.size()).c_str());

        latency_cac->set_tx_timestamp(flow_track->extract_sn(bytes), time_stamp);
    }

    void on_lm_pkt_gen (const vector<UINT8> &sn)
    {
        PACKETS pkts;
        FIELD_BLOCK pkt(tx_pkt);

        for(UINT8 i=0; i<sn.size(); i++) {
            pkt.block(tx_flow_track) = flow_track->sn_bytes(sn[i]);
            pkts.push_back((PACKET) pkt);
        }

        tx_latency->insert(pkts);
    }

    void lm_start()
    {
        //in case of failure, resource would be free
        lm_stop();

        ENSURE(is_trx_started,
               "transmit not started yet");

        shared_ptr<LATENCY_CAC> ptr_cac(new LATENCY_CAC());

        shared_ptr<RX_LATENCY> ptr_rx(
                new RX_LATENCY(rx_ports,
                               make_udf_rule(rx_track_offset, flow_track->sn_match()),
                               bind(&IMPL::on_lm_rx, this, placeholders::_1, placeholders::_2,placeholders::_3)));

        shared_ptr<TX_LATENCY> ptr_tx (
                new TX_LATENCY(tx, bind(&IMPL::on_lm_tx, this, placeholders::_1, placeholders::_2,placeholders::_3)));

        ptr_cac->start(bind(&IMPL::on_lm_pkt_gen, this, placeholders::_1));

        latency_cac  = ptr_cac;
        rx_latency   = ptr_rx;
        tx_latency   = ptr_tx;

        is_lm_started = true;
    }

    void lm_stop()
    {
        if(is_lm_started) {

            tx_latency.reset();
            rx_latency.reset();

            latency_cac.reset();

            is_lm_started = false;
        }
    }

    int lm_get()
    {
        if(is_lm_started) {
            return latency_cac->get_latency();
        } else {
            return LATENCY_INVALID;
        }
    }

    void on_pkt_rx(UINT8 unit, const PACKET &pkt, UINT32 time_stamp)
    {
        if(is_snooping && snoop_pkts.size()<max_snoop_pkt) {
            SNOOP_PKT snoop_pkt;
            snoop_pkt.time_stamp  = time_stamp;
            snoop_pkt.pkt = pkt;

            snoop_pkts.push_back(snoop_pkt);
        }
    }

    void snoop_start(UINT16 max_pkt)
    {
        if(is_trx_started && !(is_snooping)) {

            max_snoop_pkt = max_pkt;
            snoop_pkts.clear();

            rx->snoop_start(bind(&IMPL::on_pkt_rx, this, placeholders::_1, placeholders::_2,placeholders::_3),
                            max_pkt);
            is_snooping = true;
        }
    }

    void snoop_stop()
    {
        if(is_trx_started && is_snooping) {
            rx->snoop_stop();
            is_snooping = false;
        }
    }

    void snoop_collect(SNOOP_PKTS &copies)
    {
        if(is_trx_started) {
            if(snoop_pkts.size()>0) {
                copy(snoop_pkts.begin(), snoop_pkts.end(), back_inserter(copies));
                snoop_pkts.clear();
            }
        }
    }

    TRX_CNT counter(bool clear)
    {
        if(is_trx_started) {
            TRX_CNT current, delta;
            current.tx = tx->counter(false);
            current.rx = rx->counter(false);
            delta = current - cnt_base;

            if(clear) {
                cnt_base = current;
            }
            return delta;
        } else {
            if(clear) {
                cnt_base = TRX_CNT();
            }
            return cnt_base;
        }
    }

    void cnt_rate_cac() //it is used as template argument, must be defined before template specialization
    {
        TRX_CNT current;
        current.tx= tx->counter(false);
        current.rx= rx->counter(false);

        static float second = (float)RATE_CAC_INTERVAL/1000;
        cnt_rate = (current - cnt_rate_prev)/second;
        cnt_rate_prev = current;
    }


    TRX_CNT counter_rate()
    {
        return cnt_rate;
    }

    TRX_FLOW::INFO info() const
    {
        TRX_FLOW::INFO r;
        r.tx_port = tx_port;
        r.tx_pkt  = tx_pkt.name();
        r.tx_rate = tx_rate;
        r.tx_burst = tx_burst;

        r.rx_ports = rx_ports;
        r.rx_track_offset = rx_track_offset;

        r.is_trx_started = is_trx_started;
        r.is_snooping = is_snooping;
        r.is_lm_started = is_lm_started;

        return r;
    }
};


TRX_FLOW::TRX_FLOW()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    pimpl = new IMPL();

    EXP_RECAP_END;
}

TRX_FLOW::~TRX_FLOW()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    SAFE_DELETE(pimpl);

    EXP_RECAP_END;
}

void TRX_FLOW::set_tx(const PORT_NAME &tx_port)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    ENSURE(PORT_IMPL::is_valid(tx_port), "invalid tx port");

    pimpl->tx_port = tx_port;

    EXP_RECAP_END;
}

void TRX_FLOW::set_tx(const FIELD_BLOCK &tx_pkt, const FIELD::INDEX &tx_flow_track)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    ENSURE(0 == (tx_pkt.offset_of_bit(tx_flow_track)%8),
            "flow track is placed in wrong position, must be byte line up");

    pimpl->tx_pkt.reset(tx_pkt);
    pimpl->tx_flow_track = tx_flow_track;
    pimpl->tx_track_offset = tx_pkt.offset_of_bit(tx_flow_track)/8;

    EXP_RECAP_END;
}

void TRX_FLOW::set_tx(const RATE &tx_rate, const BURST &tx_burst)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    ENSURE(tx_rate.value>0, "tx rate is zero");

    pimpl->tx_rate = tx_rate;
    pimpl->tx_burst = tx_burst;

    EXP_RECAP_END;
}

void TRX_FLOW::set_rx(const PORT_NAME_SET &rx_ports)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    ENSURE(PORT_IMPL::is_valid(rx_ports), "rx port invalid");
    ENSURE(is_same_unit(rx_ports), "rx port should in same unit");

    pimpl->rx_ports = rx_ports;

    EXP_RECAP_END;
}

void TRX_FLOW::set_rx(UINT32 offset)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    pimpl->rx_track_offset = offset;

    EXP_RECAP_END;
}


void TRX_FLOW::trx_start()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ENSURE_INIT_OK;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    pimpl->trx_start();

    EXP_RECAP_END;
}

void TRX_FLOW::trx_stop(void)
{
    EXP_RECAP_START;

    AUTO_MUTEX_API;

    AUTO_TRC_FUNC;

    ASSERT(pimpl);

    pimpl->trx_stop();

    EXP_RECAP_END;

}

void TRX_FLOW::snoop(unsigned short max_pkt)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    if(max_pkt > 0) {
        pimpl->snoop_start(max_pkt);
    } else {
        pimpl->snoop_stop();
    }

    EXP_RECAP_END;

}

void TRX_FLOW::dump(SNOOP_PKTS &copies)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    pimpl->snoop_collect(copies);

    EXP_RECAP_END;
}


void TRX_FLOW::lm_start(void)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);
    pimpl->lm_start();

    EXP_RECAP_END;

}

void TRX_FLOW::lm_stop(void)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);
    pimpl->lm_stop();

    EXP_RECAP_END;
}

int TRX_FLOW::lm_get()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    return pimpl->lm_get();

    EXP_RECAP_END;
}


TRX_CNT TRX_FLOW::counter(bool clear)
{
    EXP_RECAP_START

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    return pimpl->counter(clear);

    EXP_RECAP_END
}

TRX_CNT TRX_FLOW::counter_rate()
{
    EXP_RECAP_START

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    return pimpl->counter_rate();

    EXP_RECAP_END
}


TRX_FLOW::INFO TRX_FLOW::info() const
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    return pimpl->info();

    EXP_RECAP_END;
}
