
#include "hal_queue.h"
#include "defs.h"

extern "C" {
#include "bcm/cosq.h"
#include "soc/mem.h"
#include "soc/drv.h"
}

#include <map>
using std::map;

void CIRCLE_FLOW::hal_q_create_shaper(UINT8 unit, UINT8 port, UINT8 cosq, UINT32 min_kbps, UINT32 max_kbps)
{
    ENSURE_OK(
        bcm_cosq_port_bandwidth_set(unit, port, cosq, min_kbps, max_kbps, 0));
}

/* get hw shaper bucket refresh value (rate),
 * which was configured by bcm_cosq_port_bandwidth_set
 * refer cosq_port_bandwidth_set for implementation*/
TOKEN_REFRESH CIRCLE_FLOW::hal_q_get_shaper(UINT8 unit, UINT8 port, UINT8 cosq)
{
    TOKEN_REFRESH token;

#if defined(CF_BCM_56334)

    uint64 regval_64;

    ENSURE_OK(
        READ_MAXBUCKETCONFIG_64r(unit, port, cosq, &regval_64));
    token.max = soc_reg64_field32_get(unit,
                                      MAXBUCKETCONFIG_64r,
                                      regval_64,
                                      MAX_REFRESHf);

    ENSURE_OK(
        READ_MINBUCKETCONFIG_64r(unit, port, cosq, &regval_64));
    token.min = soc_reg64_field32_get(unit,
                                      MINBUCKETCONFIG_64r,
                                      regval_64,
                                      MIN_REFRESHf);

#else
    "chip type not suppoted yet, pls complete it here."
#endif

    return token;
}


/* set refresh (rate), and keep bucket counter as zero, to avoid burst
 * refer cosq_port_bandwidth_set for implementation*/
void CIRCLE_FLOW::hal_q_set_shaper(UINT8 unit, UINT8 port, UINT8 cosq,TOKEN_REFRESH token)
{
#if defined(CF_BCM_56334)

    uint64 regval_64;
    uint32 regval_32;

    ENSURE_OK(
        READ_MAXBUCKETCONFIG_64r(unit, port, cosq, &regval_64));
    soc_reg64_field32_set(unit, MAXBUCKETCONFIG_64r, &regval_64, MAX_REFRESHf, token.max);
    ENSURE_OK(
        WRITE_MAXBUCKETCONFIG_64r(unit, port, cosq, regval_64));

    ENSURE_OK(
        READ_MINBUCKETCONFIG_64r(unit, port, cosq, &regval_64));
    soc_reg64_field32_set(unit, MINBUCKETCONFIG_64r, &regval_64, MIN_REFRESHf, token.min);
    ENSURE_OK(
        WRITE_MINBUCKETCONFIG_64r(unit, port, cosq, regval_64));

#define ZERO_BUCKET(refresh) ((refresh<<16)+1)  //not bucket dosen't mean 0

    ENSURE_OK(
        READ_MAXBUCKETr(unit, port, cosq, &regval_32));
    soc_reg_field_set(unit,
                      MAXBUCKETr,
                      &regval_32,
                      MAX_BUCKETf,
                      ZERO_BUCKET(token.max));
    soc_reg_field_set(unit, MAXBUCKETr, &regval_32, OUT_PROFILE_FLAGf, 0);
    ENSURE_OK(
        WRITE_MAXBUCKETr(unit, port, cosq, regval_32));

    ENSURE_OK(
        READ_MINBUCKETr(unit, port, cosq, &regval_32));
    soc_reg_field_set(unit,
                      MINBUCKETr,
                      &regval_32,
                      MIN_BUCKETf,
                      ZERO_BUCKET(token.min));
    soc_reg_field_set(unit, MINBUCKETr, &regval_32, OUT_PROFILE_FLAGf, 0);
    ENSURE_OK(
        WRITE_MINBUCKETr(unit, port, cosq, regval_32));

#else
    "chip type not suppoted yet, pls complete it here."
#endif
}


COUNTER CIRCLE_FLOW::hal_q_get_counter(UINT8 unit, UINT8 port, UINT8 qid, bool clear)
{
    COUNTER cnt;

#if 1
    uint64 val;

    /* because sw counter are updated periodically
     * trigger sdk counter collection first*/

    ENSURE_OK(soc_counter_sync(unit));

    ENSURE_OK(
        soc_counter_get(unit, port, SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_PKT, qid, &val));
    cnt.pkt = val;

    ENSURE_OK(
        soc_counter_get(unit, port, SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_BYTE, qid, &val));
    cnt.byte = val;

    if(clear) {
        ENSURE_OK(
            soc_counter_set(unit, port, SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_PKT, qid, 0));

        ENSURE_OK(
            soc_counter_set(unit, port, SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_BYTE, qid, 0));
    }

#else

#if defined(CF_BCM_56334)

    /* soc_counter_sync would consume some time
     * for efficiency, directly fetch value from hw*/

    int index=0;
    if(IS_GE_PORT(unit, port)) {
        index = (port-2)*8 + 56 + qid;
    } else if(IS_XE_PORT(unit, port) || IS_HG_PORT(unit, port)) {
        index = (port-26)*24 + 248 + qid;
    } else {
        ERROR("unkown port type, unit=%d, port=%d", unit, port);
    }

    egr_perq_xmt_counters_entry_t entry;
    ENSURE_OK(
        READ_EGR_PERQ_XMT_COUNTERSm(unit, MEM_BLOCK_ANY, index, &entry));

    cnt.pkt  = soc_EGR_PERQ_XMT_COUNTERSm_field32_get(unit, &entry, PACKET_COUNTERf);
    cnt.byte = soc_EGR_PERQ_XMT_COUNTERSm_field32_get(unit, &entry, BYTE_COUNTERf);

    if(clear) {
        soc_EGR_PERQ_XMT_COUNTERSm_field32_set(unit, &entry, PACKET_COUNTERf, 0);
        soc_EGR_PERQ_XMT_COUNTERSm_field32_set(unit, &entry, BYTE_COUNTERf, 0);

        ENSURE_OK(
            WRITE_EGR_PERQ_XMT_COUNTERSm(unit, MEM_BLOCK_ALL, index, &entry));
    }

#else
    "chip type not suppoted yet, pls complete it here."
#endif

#endif

    return cnt;
}

/* no SDK API available */
bool CIRCLE_FLOW::hal_q_is_empty(UINT8 unit, UINT8 port, UINT8 qid)
{
    UINT32 cnt_cell=0;

#if defined(CF_BCM_56334)
    ENSURE_OK(
        READ_OP_QUEUE_TOTAL_COUNT_CELLr(unit, port, qid, &cnt_cell));
#else
    "chip type not suppoted yet, pls complete it here."
#endif

    return cnt_cell==0 ;
}

/* refer _soc_counter_num_cosq_init */
UINT8 CIRCLE_FLOW::hal_q_num(UINT8 unit, UINT8 port_id)
{
    static map<UINT8, UINT8> num_cosq;

#if defined(CF_BCM_56334)

    static bool initialized = false;

    if(false == initialized) {
        int port;
        PBMP_ALL_ITER(unit, port){
            if (IS_CPU_PORT(unit, port)) {
                num_cosq[port] = 48;
            } else {
                num_cosq[port] = 8;
            }
        }

        const int port_24q[4] = {26, 27, 28, 29};
        for (uint8 i=0; i<4; i++) {
            num_cosq[port_24q[i]] = 24;
        }

        initialized = true;
    }

    return num_cosq[port_id];

#else
    "chip type not suppoted yet, pls complete it here."
#endif

}
