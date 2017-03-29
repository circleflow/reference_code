
#include "hal_queue.h"
#include "hal.h"
#include "queue.h"
#include "init.h"

#include "utility/export/error.h"

#include <map>
using std::map;
using std::pair;
#include <list>
using std::list;
#include <utility>
using std::make_pair;
#include <algorithm>
using std::find;

typedef list<UINT8> DB_Q;
typedef map< pair<UINT8, UINT8>, list<UINT8> > DB_PORT_Q;
static DB_PORT_Q db_port_q;

#define MIN_Q_ID 0
void CIRCLE_FLOW::queue_init(void)
{
    UINT8 unit, port;

    UNIT_ALL_ITER(unit) {

        PORT_E_ALL_ITER(unit, port) {

            for(UINT8 q=MIN_Q_ID; q<QUEUE::num_of_q(unit, port); q++) {
                db_port_q[make_pair(unit, port)].push_back(q);
            }
        }
    }
}

UINT8 QUEUE::num_of_q(UINT8 unit, UINT8 port)
{
    return hal_q_num(unit, port);
}

struct QUEUE::IMPL {

    UINT8 unit;
    UINT8 port;
    UINT8 cosq;
    TOKEN_REFRESH token;

    IMPL(UINT8 _unit, UINT8 _port)
    : unit(_unit), port(_port)
    {
        DB_PORT_Q::iterator it = db_port_q.find(make_pair(unit, port));
        ASSERT (it != db_port_q.end());

        if (! it->second.empty()) {
            cosq = it->second.back();
            it->second.pop_back();
        } else {
            ERROR("all queues used up");
        }
    }

    IMPL(UINT8 _unit, UINT8 _port, UINT8 _cosq)
    : unit(_unit), port(_port), cosq(_cosq)
    {
        DB_PORT_Q::iterator it_p = db_port_q.find(make_pair(unit, port));
        ASSERT (it_p != db_port_q.end());

        DB_Q &db_q = it_p->second;
        DB_Q::iterator it_q = find(db_q.begin(), db_q.end(), cosq);
        if(it_q != db_q.end()) {
            db_q.erase(it_q);
        } else {
            ERROR("specified cosq has been used");
        }
    }

    ~IMPL()
    {
        DB_PORT_Q::iterator it = db_port_q.find(make_pair(unit,port));

        ASSERT (it != db_port_q.end());
        it->second.push_front(cosq);
    }

    static
    RATE cvt_kbps(const RATE &_rate)
    {
        RATE rate;

        switch(_rate.type)
        {
            case RATE::KBITS_PER_SECOND:
                rate.value = _rate.value;
                break;
            case RATE::MBITS_PER_SECOND:
                rate.value = _rate.value * 1024;
                break;
            case RATE::GBITS_PER_SECOND:
                rate.value = _rate.value * 1024 * 1024;
                break;
            default:
                ERROR("invalid type");
        }

        rate.type = RATE::KBITS_PER_SECOND;

        return rate;
    }

    void shaper(const RATE &_rate)
    {
        RATE rate = cvt_kbps(_rate);

        hal_q_create_shaper(unit, port, cosq, rate.value, rate.value);

        //save the hw shaper parameter
        token = hal_q_get_shaper(unit, port, cosq);
    }

    void stop(void)
    {
        hal_q_set_shaper(unit, port, cosq, TOKEN_REFRESH());
    }

    void start(void)
    {
        hal_q_set_shaper(unit, port, cosq, token);
    }

};

QUEUE::QUEUE(UINT8 unit, UINT8 port)
{
    pimpl = new IMPL(unit, port);
}

QUEUE::QUEUE(UINT8 unit, UINT8 port, UINT8 cosq)
{
    pimpl = new IMPL(unit, port, cosq);
}

QUEUE::~QUEUE()
{
    delete pimpl;
}

void QUEUE::shaper(const RATE &rate)
{
    pimpl->shaper(rate);
}

void QUEUE::stop(void)
{
    pimpl->stop();
}

void QUEUE::start(void)
{
    pimpl->start();
}

COUNTER QUEUE::counter (bool clear) const
{
    return hal_q_get_counter(pimpl->unit, pimpl->port, pimpl->cosq, clear);
}

bool QUEUE::is_empty()
{
    return hal_q_is_empty(pimpl->unit, pimpl->port, pimpl->cosq);
}

QUEUE::INFO QUEUE::info() const
{
    INFO info={pimpl->unit, pimpl->port, pimpl->cosq};
    return info;
}
