
#include "defs.h"
#include "fp_policer.h"

extern "C" {
#include "bcm/field.h"
#include "soc/drv.h"
#include "bcm_int/esw/field.h"
}

FP_POLICER::FP_POLICER(int _unit)
:unit(_unit), eid(0)
{
    bcm_policer_config_t    pol_cfg;
    bcm_policer_config_t_init(&pol_cfg);

    pol_cfg.mode=bcmPolicerModeCommitted;
    pol_cfg.flags = BCM_POLICER_COLOR_BLIND;

    ENSURE_OK(
        bcm_policer_create(unit, &pol_cfg, &pol_id));

}

FP_POLICER::~FP_POLICER()
{
    ENSURE_OK(
        bcm_policer_destroy(unit, pol_id));
}

void FP_POLICER::attach(int _eid)
{
    eid = _eid;
    ENSURE_OK(
        bcm_field_entry_policer_attach(unit, eid, 0, pol_id));
}

/*
 since SDK has no interface to set pkt mode on policer
 here's an intrusive solution, depends on SDK implementation, verified on v5.6.5~v6.3.4
 it follows _field_trx_policer_hw_update to get the hw index of policer
*/
static
int policer_hw_index(int unit, int eid, int pol_id)
{
    _field_policer_t *f_pl;
    _field_entry_t  *f_ent;
    _field_stage_t  *stage_fc;
    int meter_hw_idx;

    _bcm_field_policer_get(unit, pol_id, &f_pl);

    _field_entry_get(unit, eid, _FP_ENTRY_PRIMARY, &f_ent);

    _field_stage_control_get(unit, f_ent->fs->stage_id, &stage_fc);

    if (stage_fc->flags & _FP_STAGE_GLOBAL_METER_POOLS) {
        /* Hw index is (Pool number * 2 * Pairs in Pool + 2 * Pair number) */
        meter_hw_idx = (2 * f_pl->pool_index *
                        stage_fc->meter_pool[f_pl->pool_index]->size)  +
                        (2 * f_pl->hw_index) + 1;
    } else {
        /* Hw index is (Slice number + 2 * Pair number) */
        meter_hw_idx = stage_fc->slices[f_pl->pool_index].start_tcam_idx +
                       (2 * f_pl->hw_index) + 1;
    }

    return meter_hw_idx;
}

/* fp policer is used for burst control only
 * no rate, only burst, that is, no refresh, only bucket counter
 * there's no SDK API provided for this purpose */
static
void fp_set_policer(int unit, int eid, int pol_id, int burst)
{
#if defined(CF_BCM_56334)

    soc_field_t fields[] = {BUCKETCOUNTf,
                            BUCKETSIZEf,
                            PKTS_BYTESf,
                            METER_GRANf,
                            REFRESHCOUNTf};
    uint32 values[]      = {0,
                            0xfff,
                            1,
                            7,
                            0x7ffff};

#define TOKEN_PER_PKT  250
#define BUCKET_DEFICIT_CNT 0x3fffffff   //max is none

    if(burst > 0) {
        values[0] = burst*TOKEN_PER_PKT - 1;
        values[4] = 0;
    } else if(burst <0 ) {
        values[0] = BUCKET_DEFICIT_CNT;
        values[4] = 0;
    }

    ENSURE_OK(
        soc_mem_fields32_modify(unit, FP_METER_TABLEm,
                            policer_hw_index(unit, eid, pol_id),
                            5, fields, values));

#else
    "chip type not suppoted yet, pls complete it here."
#endif

}

static
unsigned int fp_policer_burst_max()
{
#if defined(CF_BCM_56334)
    return 1071480; //in pkt, value get from experiment, max(BUCKETCOUNTf)/250 not works
#else
    "chip type not suppoted yet, pls complete it here."
#endif
}

unsigned int FP_POLICER::max()
{
    return fp_policer_burst_max();
}

void FP_POLICER::set(int num_of_pkt)
{
    fp_set_policer(unit, eid, pol_id, num_of_pkt);
}

