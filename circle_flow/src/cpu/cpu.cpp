
#include "utility/export/smart_ptr_u.h"
#include "defs.h"
#include "cpu.h"
#include "hal.h"
#include "cpu_hal.h"
#include "init.h"
#include "queue.h"

#include <stack>
using std::stack;
#include <map>
using std::map;
#include <utility>
using std::make_pair;
#include <algorithm>
using std::copy;

using namespace CIRCLE_FLOW::CPU;

/* once a pkt received from SDK, it should be able to find the corresponding observer to call back
 * there's several choice to define the mapping method, and they could be combined.
 * > FP match ID
 * > source port
 * > CPU queue
 * the simple choice is "match ID + source port", in this case, only one CPU queue is required,
 * thus no need to manage the CPU queue resource.
 * but according to experiment and confirmation from BCM, the match ID action cannot works together with timestamp action.
 * then the available choice is: "source port + cpu queue"
 *
 * we need to manage the cpu queue, and allocate a unique "source port + queue" for every RX instance
 * each observer instance need to bind with a RX instance, to establish a mapping "source port + queue" and observer instance.
 * */

typedef vector<bool>        DB_Q;     //cpu queue allocation, true: used, false: unused
typedef map<UINT8, DB_Q>    DB_PQ;    //port + queue allocation
typedef map<UINT8, DB_PQ>   DB_UPQ;   //unit + port + queue allocation

static DB_UPQ db_upq;
static UINT8 min_cpu_q, max_cpu_q;

#define INVALID_QID 254

static
void db_init(UINT8 num_of_cpu_q, const vector<UINT8> &ports_all, const vector<UINT8> &units_all)
{
    ASSERT(num_of_cpu_q > 0);
    ASSERT(num_of_cpu_q <= INVALID_QID);

    ASSERT(ports_all.size() > 0);
    ASSERT(units_all.size() > 0);

    min_cpu_q = 0;
    max_cpu_q = num_of_cpu_q - 1;

    DB_Q db_q;
    db_q.resize(num_of_cpu_q, false);

    DB_PQ db_pq;
    for(vector<UINT8>::const_iterator it=ports_all.begin(); it!=ports_all.end(); it++) {
        db_pq.insert(make_pair(*it, db_q));
    }

    for(vector<UINT8>::const_iterator it=units_all.begin(); it!=units_all.end(); it++) {
        db_upq.insert(make_pair(*it, db_pq));
    }
}

/*
 * to maxium queue utilization, prefer the queue which is most reaching of full
 */
static
UINT8 prefer_factor(const DB_PQ &db_pq, UINT8 q)
{
    UINT8 factor = 0;

    for(DB_PQ::const_iterator it=db_pq.begin(); it!=db_pq.end(); it++) {
        const DB_Q &db_q = it->second;
        factor += (db_q[q]==true ? 1 : 0);
    }

    return factor;
}

static
UINT8 db_alloc(UINT8 unit, const vector<UINT8> &ports)
{
    UINT8 q_select = INVALID_QID;
    UINT8 max_factor=0;

    for(UINT8 q=min_cpu_q; q<=max_cpu_q; q++) {

        ASSERT( db_upq.find(unit)!=db_upq.end() );

        DB_PQ &db_pq = db_upq[unit];

        bool  available = true;
        for(vector<UINT8>::const_iterator it=ports.begin(); it!=ports.end(); it++) {

            UINT8 port = *it;
            ASSERT(db_pq.find(port) != db_pq.end());

            DB_Q &db_q = db_pq[port];

            if(true == db_q[q]) {
                available = false;
                break;
            }
        }

        if(available) {
            UINT8 factor = prefer_factor(db_pq, q);

            if(INVALID_QID == q_select) {
                q_select = q;
                max_factor = factor;
            } else if(factor > max_factor) {
                q_select = q;
                max_factor = factor;
            }
        }
    }

    if(INVALID_QID != q_select) {
        for(vector<UINT8>::const_iterator it=ports.begin(); it!=ports.end(); it++) {
            UINT8 port = *it;
            DB_PQ &db_pq = db_upq[unit];
            DB_Q &db_q = db_pq[port];

            db_q[q_select] = true;
        }
    }

    return q_select;
}

static
void db_free(UINT8 unit, const vector<UINT8> &ports, UINT8 q)
{
    ASSERT(q<=max_cpu_q);

    ASSERT( db_upq.find(unit)!=db_upq.end() );

    DB_PQ &db_pq = db_upq[unit];

    for(vector<UINT8>::const_iterator it=ports.begin(); it!=ports.end(); it++) {

        UINT8 port = *it;
        ASSERT(db_pq.find(port) != db_pq.end());

        DB_Q &db_q = db_pq[port];

        ASSERT(true == db_q[q]);

        db_q[q] = false;
    }
}

static void rx_dispatch(UINT8 unit, UINT8 cosq, UINT8 match_id, const PACKET &pkt, UINT32 time_stamp);

void CIRCLE_FLOW::cpu_init(void)
{
    db_init(QUEUE::num_of_q(DEFAULT_UNIT, CPU_PORT_ID),
            hal_port_e_all(DEFAULT_UNIT),
            hal_unit_all());

    hal_cpu_init(rx_dispatch);
}

class DISPATCH {
    UINT8 unit;
    UINT8 port;
    UINT8 q;

public:
    DISPATCH(UINT8 _unit, UINT8 _port, UINT8 _q) : unit(_unit), port(_port), q(_q) {}

    bool operator == (const DISPATCH &ref) const
    {
        return (unit==ref.unit) && (port==ref.port) && (q==ref.q);
    }

    bool operator < (const DISPATCH &ref) const
    {
        return make_pair(make_pair(unit, port),q)
                < make_pair(make_pair(ref.unit,ref.port),ref.q);
    }
};

typedef map< DISPATCH, RX::PKT_CB > DB_DISPATCH;
static DB_DISPATCH db_dispatch;

RX::RX(UINT8 _unit, const vector<UINT8> &_ports)
: unit(_unit), ports(_ports)
{
    ASSERT(ports.size()>0);

    qid = db_alloc(unit, ports);

    if(INVALID_QID == qid) {
        ERROR("no free CPU queue");
    }
}

void RX::bind(const PKT_CB &cb)
{
    for(vector<UINT8>::const_iterator it=ports.begin(); it!=ports.end(); it++){
        db_dispatch[DISPATCH(unit,*it,qid)] = cb;
    }
}

void RX::unbind()
{
    for(vector<UINT8>::const_iterator it=ports.begin(); it!=ports.end(); it++){
        db_dispatch.erase(DISPATCH(unit,*it,qid));
    }
}

RX::~RX()
{
    unbind();

    db_free(unit, ports, qid);
}

static void rx_dispatch(UINT8 unit, UINT8 cosq, UINT8 src_port, const PACKET &pkt, UINT32 time_stamp)
{
    TRC_PACKET("\r\n unit: %d cosq %d src_port:%d time_stamp:%d", unit, cosq, src_port, time_stamp);
    TRC_PACKET(bytes_dump(pkt, 32).c_str());

    AUTO_MUTEX_API;

    DB_DISPATCH::iterator it = db_dispatch.find(DISPATCH(unit, src_port, cosq));

    if(it != db_dispatch.end()) {
        (it->second)(unit, pkt, time_stamp);
    }
}

void CIRCLE_FLOW::CPU::tx(const QUEUE &queue, const PACKETS &pkts)
{
    hal_cpu_tx(queue, pkts);
}

