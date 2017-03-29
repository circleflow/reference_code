
#include "fp_counter.h"
#include "utility/export/error.h"

extern "C" {
#include "bcm/field.h"
}

#define NUM_STAT 2

static bcm_field_stat_t stat_items[NUM_STAT] = {bcmFieldStatPackets,bcmFieldStatBytes};

FP_COUNTER::FP_COUNTER(int _unit, int gid)
:unit(_unit)
{
    ENSURE_OK(
        bcm_field_stat_create(unit, gid, NUM_STAT, stat_items, &cnt_id));
}

FP_COUNTER::~FP_COUNTER()
{
    ENSURE_OK(
        bcm_field_stat_destroy(unit, cnt_id));
}

void FP_COUNTER::attach(int _eid)
{
    ENSURE_OK(
        bcm_field_entry_stat_attach(unit, _eid, cnt_id));
}

COUNTER FP_COUNTER::get(bool clear)
{
    COUNTER cnt;

    uint64 vals[NUM_STAT];
    ENSURE_OK(
        bcm_field_stat_multi_get(unit, cnt_id, NUM_STAT, stat_items, vals));
    cnt.pkt  = vals[0];
    cnt.byte = vals[1];

    if(clear) {
        for(UINT8 i=0; i<NUM_STAT; i++) {
            ENSURE_OK(
                bcm_field_stat_set(unit, cnt_id, stat_items[i], 0));
        }
    }

    return cnt;
}

