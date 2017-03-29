
#include "hal_udf.h"
#include "defs.h"
#include "qual.h"
#include "hal.h"
#include "bytes_bits.h"

#include <map>
using std::map;
#include <utility>
using std::pair;
using std::make_pair;

extern "C" {
#include "bcm/field.h"
#include "bcm/types.h"
#include "soc/drv.h"
#include "soc/mem.h"
}

static
int _create_udf(int unit, int offset, int length)
{
    bcm_field_data_qualifier_t      data_qual;
    bcm_field_data_packet_format_t  pkt_fmt;

    bcm_field_data_qualifier_t_init( &data_qual );

    data_qual.offset_base = bcmFieldDataOffsetBasePacketStart;
    data_qual.offset      = offset;
    data_qual.length      = length;

    ENSURE_OK(
        bcm_field_data_qualifier_create( unit, &data_qual ));

    bcm_field_data_packet_format_t_init( &pkt_fmt );

    pkt_fmt.l2       = BCM_FIELD_DATA_FORMAT_L2_ANY;
    pkt_fmt.vlan_tag = BCM_FIELD_DATA_FORMAT_VLAN_TAG_ANY;
    pkt_fmt.outer_ip = BCM_FIELD_DATA_FORMAT_IP_ANY;
    pkt_fmt.inner_ip = BCM_FIELD_DATA_FORMAT_IP_ANY;
    pkt_fmt.tunnel   = BCM_FIELD_DATA_FORMAT_TUNNEL_ANY;
    pkt_fmt.mpls     = BCM_FIELD_DATA_FORMAT_MPLS_ANY;

    ENSURE_OK(
        bcm_field_data_qualifier_packet_format_add( unit, data_qual.qual_id, &pkt_fmt ));

    return data_qual.qual_id;
}

#define LEN_OF_BLOCK hal_udf_specs().len_of_block
#define NUM_OF_BLOCK hal_udf_specs().num_of_block

static map<pair<UINT8,UINT8>, int> db_udf_id;

static
int UDF_ID(UINT8 unit, UINT8 udf_idx)
{
    return db_udf_id[make_pair(unit, udf_idx)];
}

namespace CIRCLE_FLOW {
    void _hal_udf_init(int unit)
    {
        int offset = 14;    //after ethernet header

        for(int i = 0; i<NUM_OF_BLOCK; i++) {
            db_udf_id[make_pair(unit, i)] = _create_udf(unit, offset, LEN_OF_BLOCK);
            offset += LEN_OF_BLOCK;
        }
    }
}

void CIRCLE_FLOW::hal_udf_qset(int unit, void *qset)
{
    for(int i = 0; i<NUM_OF_BLOCK; i++) {
        ENSURE_OK(
            bcm_field_qset_data_qualifier_add(unit, (bcm_field_qset_t *)qset, UDF_ID(unit, i)));
    }

}


#if defined(CF_BCM_56334)

/* the chunk is fixed to offset 2,6,10...126
 * refer chip manual */
static
int enduro_chunk_aligned_offset(int offset)
{
    int aligned;

    if((offset%4)>=2) {
        aligned = (offset/4)*4+2;
    } else {
        aligned = (offset/4)*4-2;
    }

    return aligned;
}

/* convert byte offset to chunk index
 * refer _field_fb_packet_format_offset_adjust */
static
int enduro_chunk_hw_offset(int aligned_offset)
{
    if(aligned_offset == 126) {
        return 0;
    } else {
        return (aligned_offset+2)/4;
    }
}

#endif

const UDF_HW_SPECS & CIRCLE_FLOW::hal_udf_specs(UINT8 unit)
{
    static UDF_HW_SPECS specs;
    static bool initialized = false;

    if(false == initialized) {

#if defined(CF_BCM_56334)

        specs.num_of_block = 2;
        specs.len_of_block = 16;
        specs.num_of_chunk = 8;
        specs.len_of_chunk = 4;

        specs.max_offset = 127;

        specs.chunk_aligned_offset = enduro_chunk_aligned_offset;

        initialized = true;
#else
    "chip type not suppoted yet, pls complete it here."
#endif

    }

    return specs;
}

/* refer _bcm_field_*_udf_write */
void CIRCLE_FLOW::hal_udf_chunk_set(UINT8 unit, UINT8 chunk_idx, UINT8 offset)
{
#if defined(CF_BCM_56334)

    ASSERT(chunk_idx<=7);

    UINT8 hw_offset = enduro_chunk_hw_offset(offset);
    ASSERT(hw_offset<=31) ;
    static soc_field_t fields[8] = {UDF1_OFFSET0f,
                                    UDF1_OFFSET1f,
                                    UDF1_OFFSET2f,
                                    UDF1_OFFSET3f,
                                    UDF2_OFFSET0f,
                                    UDF2_OFFSET1f,
                                    UDF2_OFFSET2f,
                                    UDF2_OFFSET3f};

    uint32  entry[SOC_MAX_MEM_FIELD_WORDS];

    ENSURE_OK(
        soc_mem_read(unit, FP_UDF_OFFSETm, MEM_BLOCK_ANY, 0, entry));

    uint32 val = hw_offset;
    soc_mem_field_set(unit,
                      FP_UDF_OFFSETm,
                      entry,
                      fields[chunk_idx],
                      &val);

    /* soc_mem_index_min(unit, FP_UDF_OFFSETm) will cause unaligned exception
     * while it do works in SDK, possibly caused by different compiler
     * SDK use C compiler whiler here is C++.
     * */
    int idx_min = 0;    //soc_mem_index_min(unit, FP_UDF_OFFSETm)
    int idx_max = soc_mem_index_max(unit, FP_UDF_OFFSETm);

    for(int index=idx_min; index<=idx_max; index++){
        ENSURE_OK(
            soc_mem_write(unit,
                          FP_UDF_OFFSETm,
                          MEM_BLOCK_ALL,
                          index,
                          entry));
    }
#else
    "chip type not suppoted yet, pls complete it here."
#endif

}

static
bool is_zero(vector<UINT8> v)
{
    for(UINT8 i=0; i<v.size(); i++) {
        if(0 != v[i]) return false;
    }

    return true;
}

void CIRCLE_FLOW::hal_udf_qualify(int unit, int eid, const QUAL::MATCH &match)
{
    if(match.value.empty()) return;

    QUAL::MATCH block_match;

    for(int i = 0; i<NUM_OF_BLOCK; i++) {

        block_match.value.assign(match.value.begin()+i*LEN_OF_BLOCK, match.value.begin()+(i+1)*LEN_OF_BLOCK);
        block_match.mask.assign (match.mask.begin() +i*LEN_OF_BLOCK, match.mask.begin() +(i+1)*LEN_OF_BLOCK);

        if(!is_zero(block_match.mask)) {
            BYTES_CONVERT val(block_match.value);
            BYTES_CONVERT mask(block_match.mask);

            ENSURE_OK(
                bcm_field_qualify_data(unit, eid, UDF_ID(unit,i), val, mask, LEN_OF_BLOCK));
        }
    }
}
