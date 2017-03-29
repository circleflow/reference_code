
#include "utility/export/smart_ptr_u.h"
#include "defs.h"
#include "port_impl.h"
#include "flow.h"
#include "tx.h"
#include "rx.h"
#include "device.h"

#include <algorithm>
using std::find;


class TX_FLOW::IMPL {
public:
    PORT_NAME port;
    PACKETS   pkts;
    RATE  rate;
    BURST burst;
    shared_ptr<TX> tx;

    shared_ptr<PORT_IMPL::PAIR_CB> pair_notify;
    shared_ptr<PORT_IMPL::LINK_CB> link_notify;

    COUNTER cnt_base;
    bool is_started;

    TIMER_SAFE auto_stoper;
    TIMER_SAFE cnt_rate_cacer;

    COUNTER cnt_burst_prev;
    COUNTER cnt_rate_prev;
    COUNTER cnt_rate;

    IMPL()
    : rate(RATE::KBITS_PER_SECOND, 1000),
      burst(BURST::PKT, 0),
      is_started(false)
    {
        auto_stoper.set_op(bind(&IMPL::auto_stop, this));
        cnt_rate_cacer.set_op(bind(&IMPL::cnt_rate_cac, this));
    }

    void start(void)
    {
        stop();

        ENSURE(PORT_IMPL::is_valid(port), "invalid port");
        ENSURE(pkts.size()>0, "pkt size zero");
        ENSURE(true == port_link(PORT_IMPL::get_front(port)), "port link down");

        //local ptr, in case of failure, resource would be free automatically
        shared_ptr<TX> ptr_tx( new TX(port));

        ptr_tx->counter(true);

        ptr_tx->start(pkts, rate, burst);

        shared_ptr<PORT_IMPL::PAIR_CB> ptr_pair(new PORT_IMPL::PAIR_CB);
        *ptr_pair = bind(&IMPL::on_pair_change, this, placeholders::_1, placeholders::_2, placeholders::_3);
        pair_notify_bind(ptr_pair, make_ports(port), false);

        shared_ptr<PORT_IMPL::LINK_CB> ptr_link(new PORT_IMPL::LINK_CB);
        *ptr_link = bind(&IMPL::on_link_change, this, placeholders::_1, placeholders::_2);
        link_notify_bind(ptr_link, make_ports(port), false);

        //sw counter reset
        cnt_base = COUNTER();
        cnt_burst_prev = COUNTER();
        cnt_rate = COUNTER();
        cnt_rate_prev = COUNTER();

        if(burst.value > 0) {
            auto_stoper.start(3000, true);
        }
        cnt_rate_cacer.start(RATE_CAC_INTERVAL, true);

        tx = ptr_tx;
        pair_notify = ptr_pair;
        link_notify = ptr_link;

        is_started = true;
    }

    void stop(void)
    {
        if(is_started) {

            link_notify.reset();
            pair_notify.reset();

            auto_stoper.stop();
            cnt_rate_cacer.stop();
            cnt_rate = COUNTER();

            tx->stop();
            cnt_base = counter(true);   //save cnt value
            tx.reset();

            is_started = false;
        }
    }

    void on_link_change (const PORT_NAME &port, bool link)
    {
        if(false==link) stop();
    }

    void on_pair_change(const PORT_NAME &, const PORT_ID &, const PORT_ID &)
    {
        stop();
    }

    COUNTER counter(bool clear)
    {
        if(is_started) {
            COUNTER current = tx->counter(false);
            COUNTER delta = current - cnt_base;
            if(clear) {
                cnt_base = current;
            }
            return delta;
        } else {
            if(clear) {
                cnt_base = COUNTER();
            }

            return cnt_base;
        }
    }

    void auto_stop()
    {
        COUNTER current = tx->counter(false);
        if(cnt_burst_prev == current) {
            stop();
        } else {
            cnt_burst_prev = current;
        }
    }

    void cnt_rate_cac()
    {
        COUNTER current = tx->counter(false);
        static float second = (float)RATE_CAC_INTERVAL/1000;
        cnt_rate = (current - cnt_rate_prev)/second;
        cnt_rate_prev = current;
    }

    COUNTER counter_rate()
    {
        return cnt_rate;
    }

    TX_FLOW::INFO info(void) const
    {
        TX_FLOW::INFO data;
        data.port = port;
        data.rate = rate;
        data.burst = burst;
        data.is_started = is_started;

        return data;
    }
};

TX_FLOW::TX_FLOW()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    pimpl = new IMPL();

    EXP_RECAP_END;
}

TX_FLOW::~TX_FLOW()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    SAFE_DELETE(pimpl);

    EXP_RECAP_END;
}

void TX_FLOW::set(const PORT_NAME &tx_port)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    ENSURE(PORT_IMPL::is_valid(tx_port), "port invalid");

    pimpl->port = tx_port;

    EXP_RECAP_END;
}

void TX_FLOW::set(const PACKETS &pkts)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);
    ENSURE(pkts.size()>0, "pkt size zero");

    pimpl->pkts = pkts;

    EXP_RECAP_END;
}

void TX_FLOW::set(const RATE &rate, const BURST &burst)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);
    ENSURE(rate.value>0, "rate zero");

    pimpl->rate = rate;
    pimpl->burst = burst;

    EXP_RECAP_END;
}

void TX_FLOW::start()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ENSURE_INIT_OK;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    pimpl->start();

    EXP_RECAP_END;
}

void TX_FLOW::stop(void)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    pimpl->stop();

    EXP_RECAP_END;
}

COUNTER TX_FLOW::counter(bool clear)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    return pimpl->counter(clear);

    EXP_RECAP_END;
}

COUNTER TX_FLOW::counter_rate()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    return pimpl->counter_rate();

    EXP_RECAP_END;
}

TX_FLOW::INFO TX_FLOW::info(void) const
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    return pimpl->info();

    EXP_RECAP_END;
}

class RX_FLOW::IMPL {
public:

    QUAL qual;
    shared_ptr<RX> rx;

    shared_ptr<PORT_IMPL::PAIR_CB> pair_notify;

    COUNTER cnt_base;
    bool is_started;

    SNOOP_PKTS snoop_pkts;
    UINT16 max_snoop_pkt;
    bool is_snooping;

    TIMER_SAFE cnt_rate_cacer;

    COUNTER cnt_rate_prev;
    COUNTER cnt_rate;

    IMPL()
    : is_started(false),
      is_snooping(false)
    {
        cnt_rate_cacer.set_op(bind(&IMPL::cnt_rate_cac, this));
    }

    void start()
    {
        stop();

        //local ptr, in case of failure, resource would be automatically free
        shared_ptr<RX> ptr_tx(new RX(qual));

        shared_ptr<PORT_IMPL::PAIR_CB> ptr_pair(new PORT_IMPL::PAIR_CB);
        *ptr_pair = bind(&IMPL::on_pair_change, this, placeholders::_1, placeholders::_2, placeholders::_3);
        pair_notify_bind(ptr_pair, qual.in_ports, false);

        cnt_base = COUNTER();
        cnt_rate_prev = COUNTER();
        cnt_rate = COUNTER();
        cnt_rate_cacer.start(RATE_CAC_INTERVAL, true);

        rx = ptr_tx;
        pair_notify = ptr_pair;

        is_started = true;
    }

    void stop(void)
    {
        if(is_started) {

            snoop(0);

            pair_notify.reset();
            cnt_rate_cacer.stop();
            cnt_rate = COUNTER();

            cnt_base = counter(false);
            rx.reset();

            is_started = false;
        }
    }

    void on_pair_change(const PORT_NAME &, const PORT_ID &, const PORT_ID &)
    {
        stop();
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

    void snoop (UINT16 max_pkt)
    {
        if(max_pkt>0 && is_started &&(!is_snooping)) {
            snoop_pkts.clear();
            max_snoop_pkt = max_pkt;

            rx->snoop_start(bind(&IMPL::on_pkt_rx, this, placeholders::_1, placeholders::_2,placeholders::_3),
                            max_pkt);

            is_snooping = true;
        } else if(0==max_pkt && is_snooping) {
            rx->snoop_stop();
            is_snooping = false;
        }
    }

    void dump (SNOOP_PKTS &copies)
    {
        if(is_started) {
            if(snoop_pkts.size()>0) {
                copy(snoop_pkts.begin(), snoop_pkts.end(), back_inserter(copies));
                snoop_pkts.clear();
            }
        }
    }

    COUNTER counter(bool clear)
    {
        if(is_started) {
            COUNTER current = rx->counter(false);
            COUNTER delta = current - cnt_base;
            if(clear) {
                cnt_base = current;
            }
            return delta;
        } else {
            if(clear) {
                cnt_base = COUNTER();
            }

            return cnt_base;
        }
    }

    void cnt_rate_cac() //it is used as template argument, must be defined before template specialization
    {
        COUNTER current = rx->counter(false);
        static float second = (float)RATE_CAC_INTERVAL/1000;
        cnt_rate = (current - cnt_rate_prev)/second;
        cnt_rate_prev = current;
    }


    COUNTER counter_rate()
    {
        return cnt_rate;
    }

    RX_FLOW::INFO info()
    {
        RX_FLOW::INFO r;
        r.qual = qual;
        r.is_started = is_started;
        r.is_snooping = is_snooping;

        return r;
    }

};

RX_FLOW::RX_FLOW()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    pimpl = new IMPL();

    EXP_RECAP_END;
}

RX_FLOW::~RX_FLOW()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    SAFE_DELETE(pimpl);

    EXP_RECAP_END;
}

static
void validate(const QUAL &qual)
{
    ENSURE(PORT_IMPL::is_valid(qual.in_ports),
            "invalid in ports");
    ENSURE(is_same_unit(qual.in_ports),
            "in ports not on same unit");
    ENSURE(qual.hit_pri<=QUAL::MAX_HIT_PRI,
            "invalid hit priority %d", qual.hit_pri);

    for(QUAL::PDF_RULES::const_iterator it=qual.pdfs.begin(); it!=qual.pdfs.end(); it++) {
        ENSURE(it->first < QUAL::pdf_end,
                "invalid PDF %d", it->first);
    }
}

void RX_FLOW::set(const QUAL &qual)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    validate(qual);

    pimpl->qual = qual;

    EXP_RECAP_END;
}

RX_FLOW::INFO RX_FLOW::info() const
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    return pimpl->info();

    EXP_RECAP_END;
}

void RX_FLOW::start()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ENSURE_INIT_OK;

    AUTO_MUTEX_API;

    ASSERT(pimpl);
    pimpl->start();

    EXP_RECAP_END;;
}

void RX_FLOW::stop()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    pimpl->stop();

    EXP_RECAP_END;
}


void RX_FLOW::snoop(UINT16 max_pkt)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    ENSURE(max_pkt <= 512,
            "num of pkt exceed the max limitation(512)"); //no exact limitation, only concern of queue depth and CPU load

    pimpl->snoop(max_pkt);

    EXP_RECAP_END;
}

void RX_FLOW::dump(SNOOP_PKTS &copy)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    pimpl->dump(copy);

    EXP_RECAP_END;
}

COUNTER RX_FLOW::counter(bool clear)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    return pimpl->counter(clear);

    EXP_RECAP_END;
}

COUNTER RX_FLOW::counter_rate()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    ASSERT(pimpl);

    return pimpl->counter_rate();

    EXP_RECAP_END;
}
