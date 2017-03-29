#include "hal_port.h"
#include "port_impl.h"
#include "port.h"
#include "defs.h"
#include "init.h"

using namespace CIRCLE_FLOW;

#include <map>
using std::map;
using std::pair;
#include <utility>
using std::make_pair;
#include <limits>
using std::numeric_limits;
#include <algorithm>
using std::for_each;

#include "utility/export/timer.h"
using namespace UTILITY;

namespace CIRCLE_FLOW {
    template<class T>
    T max_value(const T &)
    {
        return numeric_limits<T>::max();
    }
}

COUNTER COUNTER::operator - (const COUNTER &operand) const
{
#define CAC_MEMBER(member) \
    if(this->member >= operand.member) { \
        r.member = this->member - operand.member; \
    } else { \
        r.member =  max_value(this->member) - operand.member + this->member; \
    }

    COUNTER r;

    CAC_MEMBER(pkt);
    CAC_MEMBER(byte);

#undef CAC_MEMBER

    return r;
}

COUNTER & COUNTER::operator -= (const COUNTER &operand)
{
    *this = (*this) - operand;
    return *this;
}

COUNTER COUNTER::operator +  (const COUNTER &operand) const
{
#define CAC_MEMBER(member) r.member = this->member + operand.member

    COUNTER r;

    CAC_MEMBER(pkt);
    CAC_MEMBER(byte);

#undef CAC_MEMBER

    return r;
}

COUNTER & COUNTER::operator += (const COUNTER &operand)
{
    *this = (*this) + operand;
    return *this;
}

bool COUNTER::operator == (const COUNTER &operand) const
{
    return (this->pkt==operand.pkt)
            && (this->byte==operand.byte);
}

COUNTER COUNTER::operator / (float divider)
{
    COUNTER r;

#define CAC_MEMBER(member) \
    {\
        double result = (double) this->member / (double)divider;\
        r.member = result;\
    }

    CAC_MEMBER(pkt);
    CAC_MEMBER(byte);

#undef CAC_MEMBER

    return r;
}

TRX_CNT TRX_CNT::operator - (const TRX_CNT &operand) const
{
    TRX_CNT r;

    r.tx = this->tx - operand.tx;
    r.rx = this->rx - operand.rx;

    return r;
}

TRX_CNT & TRX_CNT::operator -= (const TRX_CNT &operand)
{
    *this = (*this) - operand;
    return *this;
}

TRX_CNT TRX_CNT::operator +  (const TRX_CNT &operand) const
{
    TRX_CNT r;

    r.tx = this->tx + operand.tx;
    r.rx = this->rx + operand.rx;

    return r;
}

TRX_CNT & TRX_CNT::operator += (const TRX_CNT &operand)
{
    *this = (*this) + operand;
    return *this;
}

bool TRX_CNT::operator == (const TRX_CNT &operand) const
{
    return (this->tx==operand.tx)
            && (this->rx==operand.rx);
}

TRX_CNT TRX_CNT::operator / (float divider)
{
    TRX_CNT r;

    r.tx = this->tx/divider;
    r.rx = this->rx/divider;

    return r;
}


typedef map<PORT_NAME, TRX_CNT> PORTS_CNT;

static
void cnt_clear(PORTS_CNT *cnts, const PORT_NAME &port, const PORT_ID &front, const PORT_ID &engine)
{
    (*cnts)[port] = TRX_CNT ();
}

class CAC_RATE{
public:
    CAC_RATE( const PORT_NAME_SET &_ports, UINT32 _interval) //interval in ms
    {
        ports = _ports;
        interval = _interval;

        //reset rate once pair change
        pair_notify_bind(persistent_shared<PAIR_CB>(bind(cnt_clear, &prev_cnts, placeholders::_1, placeholders::_2, placeholders::_3)),
                         ports, false);

        timer.set_op(bind(&CAC_RATE::on_timer, this));
        timer.start(interval, true);
    }

    void on_timer(void)
    {
        AUTO_MUTEX_API;
        for(PORT_NAME_SET::iterator it=ports.begin(); it!=ports.end(); it++) {
            PORT_NAME port = *it;

            TRX_CNT cnt = PORT_IMPL::get_cnt(PORT_IMPL::get_front(port));

            static float second = (float)interval/1000;
            rates[port] = (cnt - prev_cnts[port])/second;
            prev_cnts[port] = cnt;
        }
    }

    TRX_CNT get_rate(const PORT_NAME &port)
    {
        return rates[port];
    }

private:

    TIMER timer;
    PORT_NAME_SET ports;
    PORTS_CNT prev_cnts;
    PORTS_CNT rates;
    UINT32 interval;
};

TRX_CNT CIRCLE_FLOW::PORT_IMPL::get_cnt (const PORT_ID &port)
{
    return hal_port_cnt_get(port.unit, port.index);
}

TRX_CNT CIRCLE_FLOW::PORT_IMPL::get_rate (const PORT_NAME &port)
{
    static CAC_RATE cac(PORT_IMPL::get_port_all(), 2000u);   //double time of SDK cnt collection interval

    return cac.get_rate(port);
}

/////////////////////// api ///////////////////////
TRX_CNT CIRCLE_FLOW::PORT::get_cnt(const PORT_NAME &port, bool is_reset)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ENSURE_INIT_OK;

    AUTO_MUTEX_API;

    static PORTS_CNT base_cnts;

    //reset counter once port interface changed
    static bool run_once = true;
    if(run_once) {
        pair_notify_bind(persistent_shared<PAIR_CB>(bind(cnt_clear, &base_cnts, placeholders::_1, placeholders::_2, placeholders::_3)));
        run_once = false;
    }

    TRX_CNT current = PORT_IMPL::get_cnt(PORT_IMPL::get_front(port));
    TRX_CNT delta = current - base_cnts[port];

    //change sw base cnt instead of reset hw cnt,
    //hw cnt never reset, because rate cac depend on it
    if(is_reset) {
        TRC_VERBOSE("counter reset is %s", is_reset ? "true" : "false");
        base_cnts[port] = current;
    }

    return delta;

    EXP_RECAP_END;
}

TRX_CNT CIRCLE_FLOW::PORT::get_rate(const PORT_NAME &port)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ENSURE_INIT_OK;

    AUTO_MUTEX_API;

    return PORT_IMPL::get_rate(port);

    EXP_RECAP_END;
}


