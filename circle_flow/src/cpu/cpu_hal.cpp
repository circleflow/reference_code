
#include "defs.h"
#include "cpu_hal.h"
#include "hal.h"
#include "sdk.h"

#include <algorithm>
using std::copy;

extern "C" {
#include "bcm/pkt.h"
#include "bcm/rx.h"
#include "bcm/cosq.h"
}


static
void cpu_sched_set(UINT8 unit)
{
    ENSURE_OK(
        bcm_cosq_port_sched_set(unit,
                                pbmp(CPU_PORT_ID),
                                BCM_COSQ_ROUND_ROBIN,
                                0,0));
}


static bcm_rx_t rx_dispatch(int unit, bcm_pkt_t *_pkt, void *cookie);
static hal_cpu_rx_cb rx_cb;

void CIRCLE_FLOW::hal_cpu_init(hal_cpu_rx_cb cb)
{
    ASSERT(cb != 0);
    rx_cb = cb;

    UINT8 unit;
    UNIT_ALL_ITER(unit) {
        cpu_sched_set(unit);

        bcm_rx_cfg_t cfg;
        bcm_rx_cfg_get(unit, &cfg); //return non-zero because not started

        cfg.pkt_size = 2048;
        for (int chan = 0; chan < BCM_RX_CHANNELS; chan++) {
            cfg.chan_cfg[chan].flags |= BCM_RX_F_OVERSIZED_OK;
        }

        ENSURE_OK(
            bcm_rx_start(unit, &cfg));

        ENSURE_OK(
            bcm_rx_queue_register(unit,
                                  "CF-RX",
                                  BCM_RX_COS_ALL,
                                  (bcm_rx_cb_f)rx_dispatch,
                                  99, //lower than packet watch
                                  0,
                                  0));
    }

}


static bcm_rx_t rx_dispatch(int unit, bcm_pkt_t *_pkt, void *cookie)
{
    /*under the sdk thread context:
      > exception should be terminated at SDK boundary
      > data race need to be avoided with other thread
    */

    EXP_FREE_START;

    PACKET pkt;
    unsigned char *p_start, *p_end;
    p_start = _pkt->pkt_data[0].data;
    p_end = p_start + _pkt->pkt_len;

    if(_pkt->rx_untagged & BCM_PKT_OUTER_UNTAGGED) {
        //strip the new added outer tag
        pkt.assign(p_start, p_start+12);
        pkt.insert(pkt.end(), p_start+16, p_end);
    } else {
        pkt.assign(p_start, p_end);
    }
    // translate into us
    UINT32 time_stamp = (_pkt->rx_timestamp>>10);

    rx_cb(unit, _pkt->cos, _pkt->src_port, pkt, time_stamp);

    EXP_FREE_END_NR;

    return BCM_RX_HANDLED;
}

void CIRCLE_FLOW::hal_cpu_tx(const QUEUE &q, const PACKETS &pkts)
{
    for(UINT8 i=0; i<pkts.size(); i++) {
        bcm_pkt_t *pkt_buf = 0;

        const PACKET &pkt = pkts[i];
        int pkt_len = pkt.size();

        ENSURE_OK(
            bcm_pkt_alloc(q.info().unit, pkt_len, 0, &pkt_buf));

        copy(pkt.begin(), pkt.end(), pkt_buf->pkt_data[0].data);

        pkt_buf->flags = BCM_TX_CRC_REGEN | BCM_TX_NO_PAD;
        pkt_buf->pkt_data[0].len = pkt_len;
        pkt_buf->blk_count       = 1;
        pkt_buf->cos             = q.info().cosq;
        pkt_buf->unit            = q.info().unit;
        BCM_PKT_PORT_SET(pkt_buf, q.info().port, false, false);

        ENSURE_OK(
            bcm_tx(q.info().unit, pkt_buf, 0));

        ENSURE_OK(
            bcm_pkt_free(q.info().unit, pkt_buf));
    }
}
