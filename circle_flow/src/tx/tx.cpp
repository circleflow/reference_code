
#include "hal_tx.h"
#include "defs.h"
#include "port_impl.h"
#include "tx.h"
#include "fp_fork.h"
#include "fp_vtag.h"
#include "fp_burst_q.h"
#include "fp_tx.h"
#include "cpu.h"
#include "mmu.h"
#include "pkt_helper.h"
#include "hal.h"
#include "init.h"

#include "utility/export/wait.h"
#include "utility/export/smart_ptr_u.h"
#include <map>
using std::map;
using std::pair;
using std::make_pair;
#include <algorithm>
using std::for_each;


struct TX::IMPL {

    PORT_ID front;
    PORT_ID engine;

    QUEUE q_engine;
    QUEUE q_front;  //used for get tx counter, keep same cosq with engine port, to convenient debug

    FP_BRUST_Q_VTAG vid;

    shared_ptr<FP_BURST_COSQ> fp_burst_q;

    shared_ptr<MMU> mmu_tx;

    IMPL(PORT_ID _front, PORT_ID _engine)
    : front(_front), engine(_engine),
      q_engine(engine.unit, engine.index),
      q_front(front.unit, front.index, q_engine.info().cosq),
      vid(q_engine.info().cosq)
    { }

    void start(const PACKETS &_pkts, RATE rate, BURST burst)
    {
        stop();

        PACKETS pkts;

        insert_vtag(_pkts, pkts, vid.get_vid());

        /* consider line rate:
         * one pkt is flying on wire, another pkt is going through pipeline
         * so, at least require 2 pkt to ensure reaching line rate */

#define LINE_RATE_PKT_NUM 4

        while(pkts.size()<LINE_RATE_PKT_NUM){
            pkts.insert(pkts.end(), pkts.begin(), pkts.end());
        }

        mmu_tx.reset(new MMU(engine.unit, pkts_len(pkts)));

        q_engine.shaper(rate);

        if(burst.value > 0) {
            fp_burst_q.reset(new FP_BURST_COSQ(engine.unit,
                                               engine.index,
                                               q_engine.info().cosq,
                                               vid.get_vid()));

            fp_burst_q->burst(burst);
        }

        //stop queue before insert pkt, to ensure flow start with the specified rate
        q_engine.stop();

        insert(pkts);

        q_engine.start();
    }

    void insert(const PACKETS &pkts)
    {
        CPU::tx(q_engine, pkts);
    }

    void stop(void)
    {
        HAL_FP_TX_STOP fp_stop(engine.unit, engine.index, vid.get_vid());

#define MAX_WAIT_Q_ENGINE 350u    //350ms, 1500byte*256pkt, under 10mbps rate
#define MAX_WAIT_Q_FRONT  100u    //100ms

        //wait until queue empty
        if(!q_engine.is_empty()) {
            WAIT(MAX_WAIT_Q_ENGINE, bind(&QUEUE::is_empty,&q_engine));
            ASSERT(q_engine.is_empty());
        }

        if(!q_front.is_empty()) {
            WAIT(MAX_WAIT_Q_FRONT, bind(&QUEUE::is_empty, &q_front));
            ASSERT(q_front.is_empty());
        }
    }

    COUNTER counter(bool clear)
    {
        return q_front.counter(clear);
    }
};

TX::TX(const PORT_NAME &port)
{
    pimpl = new IMPL(PORT_IMPL::get_front(port), PORT_IMPL::get_engine(port));
}

TX::~TX()
{
    delete pimpl;
}

void TX::start(const PACKETS &pkts, RATE rate, BURST burst)
{
    pimpl->start(pkts, rate, burst);
}

void TX::stop(void)
{
    pimpl->stop();
}

COUNTER TX::counter(bool clear)
{
    return pimpl->counter(clear);
}


#include "observer.h"

struct TX_LATENCY::IMPL {

    shared_ptr<TX> tx;

    UINT8 unit;
    FP_LATENCY_VTAG vid;
    shared_ptr<MMU> mmu_tx;

    IMPL(shared_ptr<TX> _tx, const CPU::RX::PKT_CB &cb)
    : tx(_tx),
      unit(tx->pimpl->front.unit),
      vid(unit, tx->pimpl->q_engine.info().cosq)
    {
        add_observer(unit, vid.get_vid(), cb);
    }

    ~IMPL()
    {
        remove_observer(unit, vid.get_vid());
    }

    void insert(const PACKETS &_pkts)
    {
        PACKETS pkts;
        insert_vtag(_pkts, pkts, vid.get_vid());

        mmu_tx.reset(new MMU(unit, pkts_len(pkts)));

        tx->pimpl->insert(pkts);
    }
};

TX_LATENCY::TX_LATENCY(shared_ptr<TX> tx, const CPU::RX::PKT_CB &cb)
{
    pimpl = new IMPL(tx, cb);
}

TX_LATENCY::~TX_LATENCY()
{
    delete pimpl;
}

void TX_LATENCY::insert(const PACKETS &pkts)
{
    pimpl->insert(pkts);
}

//////////////////////////// init //////////////////////

//init latency loopback port
typedef map<UINT8, UINT8> DB_LATENCY_PORT;
static DB_LATENCY_PORT latency_port;

static
UINT8 get_latency_port(UINT8 unit)
{
    DB_LATENCY_PORT::iterator it= latency_port.find(unit);

    ASSERT(it != latency_port.end());

    return it->second;
}

static
void latency_init(const DEVICE::CONFIG::LATENCY_PROFILE &profile)
{
    const UINT8 &unit = profile.unit;
    const UINT8 &port = profile.port;

    ASSERT(latency_port.find(unit) == latency_port.end());  //one latency port per unit

    hal_init_latency_port(unit, port);

    //no free
    CPU::RX * rx = new CPU::RX(unit, vector<UINT8>(1,port));
    rx->bind(observer_dispatch);

    new FP_LATENCY_PORT(unit, port, rx->get_qid());

    latency_port.insert(std::make_pair(unit, port));
}

//adjust port pair once port iterface mode changed
static
void pair_init(const PORT_NAME &name, const PORT_ID &_front, const PORT_ID &_engine)
{
    typedef map<string, shared_ptr<FP_FORK_PORT> > DB_FORK;
    typedef map<string, shared_ptr<FP_FORK_LATENCY> > DB_LATENCY;

    static DB_FORK db_fork;
    static DB_LATENCY db_latency;

    ASSERT(_front.unit == _engine.unit);

    const UINT8 &unit   = _front.unit;
    const UINT8 &front  = _front.index;
    const UINT8 &engine = _engine.index;

    db_fork[name].reset();
    db_latency[name].reset();

    shared_ptr<FP_FORK_PORT> fork(new FP_FORK_PORT(unit, engine, front));
    shared_ptr<FP_FORK_LATENCY> latency(new FP_FORK_LATENCY(unit, engine, front, get_latency_port(unit)));

    hal_init_front_port(unit, front);
    hal_init_engine_port(unit, engine);

    db_fork[name] = fork;
    db_latency[name] = latency;
}

//sync front port mode to engine port mode, keep them work at same speed
static
void sync_port_mode (const PORT_NAME &port, bool link)
{
    if(link) {
        PORT_ID front = PORT_IMPL::get_front(port);
        PORT_ID engine = PORT_IMPL::get_engine(port);

        PORT::STATUS status = PORT_IMPL::get_status(front);

        PORT::MODE mode;
        mode.ds = status.ds;
        mode.pause = false;

        PORT_IMPL::set_forced(engine, mode);

        //make sure engine port link up
        if(false == port_link(engine)){
#define MAX_LINK_UP_WAIT 300u //ms
            WAIT(MAX_LINK_UP_WAIT, bind(&PORT_IMPL::port_link, engine));
        }

        ASSERT(true == port_link(engine));

    } else {
        //sdk would clean up the port queue
    }
}

void CIRCLE_FLOW::tx_init(const vector<DEVICE::CONFIG::LATENCY_PROFILE> &profiles)
{

    for_each(profiles.begin(), profiles.end(), latency_init);

    //will be notified after the any other notifiers(after flow update)
    pair_notify_bind(persistent_shared<PAIR_CB>(pair_init));

    link_notify_bind(persistent_shared<LINK_CB>(sync_port_mode));
}
