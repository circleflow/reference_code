#include "defs.h"
#include "latency_cac.h"
#include "flow_track.h"

#include "utility/export/timer.h"
#include "utility/export/unique.h"

#include <algorithm>
using std::fill;

#define NUM_OF_SN        (FLOW_TRACK::max_sn()+1)
#define MEASURE_INTERVAL 100    //ms
#define TS_INVALID       0
#define MAX_WAIT_CNT     10

static
const vector<UINT8> & sn_vector(void)
{
    static vector<UINT8> sn;
    if(0 == sn.size()) {

        for(UINT8 i=0; i<NUM_OF_SN; i++) {
            sn.push_back(i);
        }
    }

    return sn;
}

enum {
    STOP,
    START,
    WAIT_TX,
    WAIT_RX
};

static
bool is_data_complete(const vector<UINT32> &ts)
{
    for(vector<UINT32>::const_iterator it=ts.begin(); it!=ts.end(); it++) {
        if(TS_INVALID == *it) {
            return false;
        }
    }

    return true;
}

static
int cac_latency(const vector<UINT32> &tx_time, const vector<UINT32> &rx_time)
{
    UINT32 sum = 0;
    UINT8  cnt = 0;

    for(UINT8 i=0; i<rx_time.size(); i++) {
        if((TS_INVALID!=rx_time[i]) && (TS_INVALID!=tx_time[i]))
        {
            if(rx_time[i] >= tx_time[i]) {
                sum +=rx_time[i]-tx_time[i];
            } else {
                //timestamp turn over
                sum += (UINT32)0xffffffff - tx_time[i];
                sum += 1;
                sum += rx_time[i];
            }

            cnt ++;
        }
    }

    int latency;
    if(cnt) {
        latency = sum/cnt;
    } else {
        latency = LATENCY_INVALID;
    }

    return latency;
}

class LATENCY_CAC::IMPL {

public:

    TIMER_SAFE timer;
    UINT32 state;
    UINT8 rx_wait_cnt;
    UINT8 tx_wait_cnt;

    vector<UINT32> rx_time;
    vector<UINT32> tx_time;
    int latency;

    PKT_GEN pkt_gen;

    typedef UNIQUE<LATENCY_CAC, UINT16, 0, 4096> INSTANCE_ID;
    INSTANCE_ID instance_id;

    IMPL()
    : state(STOP),
      rx_time(NUM_OF_SN, TS_INVALID),
      tx_time(NUM_OF_SN, TS_INVALID),
      latency(LATENCY_INVALID)
    {
        timer.set_op(bind(&IMPL::state_machine,this));
    }

    void start(const PKT_GEN &_pkt_gen)
    {
        if(STOP == state) {

            pkt_gen = _pkt_gen;

            state = START;
            timer.start(0);
        }
    }

    void stop()
    {
        state = STOP;
        timer.stop();
    }

    void state_machine(void)
    {
        switch(state) {

        case START :
            fill(rx_time.begin(), rx_time.end(), TS_INVALID);
            fill(tx_time.begin(), tx_time.end(), TS_INVALID);

            tx_wait_cnt = 0;
            rx_wait_cnt = 0;

            (pkt_gen)(sn_vector());

            state = WAIT_TX;
            timer.start(MEASURE_INTERVAL);
            break;

        case WAIT_TX :

            tx_wait_cnt ++;

            if(tx_wait_cnt>MAX_WAIT_CNT) {
                TRC_LATENCY("\r\n latency cac tx wait time out");
            }

            if(is_data_complete(tx_time)
                    || (tx_wait_cnt>MAX_WAIT_CNT)) {
                state = WAIT_RX;
            }

            timer.start(MEASURE_INTERVAL);

            break;

        case WAIT_RX :

            rx_wait_cnt ++;

            if(rx_wait_cnt>MAX_WAIT_CNT) {
                TRC_LATENCY("\r\n latency cac rx wait time out");
            }

            if(is_data_complete(rx_time)
                    || (rx_wait_cnt>MAX_WAIT_CNT)) {
                latency = cac_latency(tx_time, rx_time);
                state = START;
            }

            timer.start(MEASURE_INTERVAL);

            break;

        case STOP:
            break;

        default:
            ERROR("unkown state %d",state);
            state = STOP;
        }
    }
};

LATENCY_CAC::LATENCY_CAC()
{
    pimpl = new IMPL();
}

LATENCY_CAC::~LATENCY_CAC()
{
    delete pimpl;
}

void LATENCY_CAC::start(const PKT_GEN &pkt_gen)
{
    pimpl->start(pkt_gen);
}

void LATENCY_CAC::stop()
{
    pimpl->stop();
}

int LATENCY_CAC::get_latency(void)
{
    return pimpl->latency;
}

void LATENCY_CAC::set_tx_timestamp(UINT8 sn, UINT32 timestamp)
{
    ASSERT(sn<NUM_OF_SN);
    pimpl->tx_time[sn]=timestamp;
}

void LATENCY_CAC::set_rx_timestamp(UINT8 sn, UINT32 timestamp)
{
    ASSERT(sn<NUM_OF_SN);
    pimpl->rx_time[sn]=timestamp;
}
