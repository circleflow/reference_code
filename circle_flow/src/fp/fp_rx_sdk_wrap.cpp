
#include "fp_rx_sdk_wrap.h"
#include "bytes_bits.h"

#include "utility/export/error.h"

#include <map>
using std::map;
using std::make_pair;

extern "C" {
#include "bcm/field.h"
}

/* wrap bcm_field_qualify_xxx API into a unified invoke way */

#define WARP(func) _wrap_##func

#define QUAL_FUC_1(func,type) \
static int WARP(func)(int unit, int eid, const QUAL::MATCH &match)\
{\
    BYTES_CONVERT val(match.value);\
    return func(unit, eid, (type)val);\
}

#define QUAL_FUC_2(func,type) \
static int WARP(func)(int unit, int eid, const QUAL::MATCH &match)\
{\
    BYTES_CONVERT val(match.value);\
    BYTES_CONVERT mask(match.mask);\
    return func(unit, eid, (type)val, (type)mask);\
}


QUAL_FUC_2(bcm_field_qualify_DstMac, uint8*)
QUAL_FUC_2(bcm_field_qualify_SrcMac, uint8*)
QUAL_FUC_2(bcm_field_qualify_EtherType,uint16)
QUAL_FUC_1(bcm_field_qualify_OuterTpid, uint16)
QUAL_FUC_2(bcm_field_qualify_OuterVlanCfi,uint8)
QUAL_FUC_2(bcm_field_qualify_OuterVlanPri,uint8)
QUAL_FUC_2(bcm_field_qualify_OuterVlanId,uint16)
QUAL_FUC_1(bcm_field_qualify_InnerTpid,uint16)
QUAL_FUC_2(bcm_field_qualify_InnerVlanCfi,uint8)
QUAL_FUC_2(bcm_field_qualify_InnerVlanPri,uint8)
QUAL_FUC_2(bcm_field_qualify_InnerVlanId,uint16)
QUAL_FUC_2(bcm_field_qualify_DstIp,uint32)
QUAL_FUC_2(bcm_field_qualify_SrcIp,uint32)
QUAL_FUC_2(bcm_field_qualify_L4DstPort,int)
QUAL_FUC_2(bcm_field_qualify_L4SrcPort,int)
QUAL_FUC_2(bcm_field_qualify_IpProtocol,uint8)
QUAL_FUC_2(bcm_field_qualify_DSCP,uint8)
QUAL_FUC_2(bcm_field_qualify_Ttl,uint8)
QUAL_FUC_2(bcm_field_qualify_TcpControl,uint8)
QUAL_FUC_2(bcm_field_qualify_ExtensionHeaderType,uint8)
QUAL_FUC_2(bcm_field_qualify_ExtensionHeaderSubCode,uint8)
QUAL_FUC_2(bcm_field_qualify_Ip6FlowLabel,uint32)
QUAL_FUC_2(bcm_field_qualify_DstIp6,uint8*)
QUAL_FUC_2(bcm_field_qualify_SrcIp6,uint8*)

typedef int (*WRAP_QUAL_FUC)(int, int, const QUAL::MATCH &);

struct FIELD_INFO {
    QUAL::PDF field;
    bcm_field_qualify_t qset;
    WRAP_QUAL_FUC func;
    FP_STAGE stage;
};

static const FIELD_INFO field_info[] =
{
    {QUAL::dst_mac,             bcmFieldQualifyDstMac,                  WARP(bcm_field_qualify_DstMac),         STAGE_IFP},
    {QUAL::src_mac,             bcmFieldQualifySrcMac,                  WARP(bcm_field_qualify_SrcMac),         STAGE_IFP},
    {QUAL::ether_type,          bcmFieldQualifyEtherType,               WARP(bcm_field_qualify_EtherType),      STAGE_IFP},
    {QUAL::outer_tpid,          bcmFieldQualifyOuterTpid,               WARP(bcm_field_qualify_OuterTpid),      STAGE_IFP},
    {QUAL::outer_vlan_cfi,      bcmFieldQualifyOuterVlanCfi,            WARP(bcm_field_qualify_OuterVlanCfi),   STAGE_IFP},
    {QUAL::outer_vlan_pri,      bcmFieldQualifyOuterVlanPri,            WARP(bcm_field_qualify_OuterVlanPri),   STAGE_IFP},
    {QUAL::outer_vlan_id,       bcmFieldQualifyOuterVlanId,             WARP(bcm_field_qualify_OuterVlanId),    STAGE_IFP},
    {QUAL::inner_tpid,          bcmFieldQualifyInnerTpid,               WARP(bcm_field_qualify_InnerTpid),      STAGE_IFP},
    {QUAL::inner_vlan_cfi,      bcmFieldQualifyInnerVlanCfi,            WARP(bcm_field_qualify_InnerVlanCfi),   STAGE_IFP},
    {QUAL::inner_vlan_pri,      bcmFieldQualifyInnerVlanPri,            WARP(bcm_field_qualify_InnerVlanPri),   STAGE_IFP},
    {QUAL::inner_vlan_id,       bcmFieldQualifyInnerVlanId,             WARP(bcm_field_qualify_InnerVlanId),    STAGE_IFP},
    {QUAL::dst_ip,              bcmFieldQualifyDstIp,                   WARP(bcm_field_qualify_DstIp),          STAGE_IFP},
    {QUAL::src_ip,              bcmFieldQualifySrcIp,                   WARP(bcm_field_qualify_SrcIp),          STAGE_IFP},
    {QUAL::l4_dst_port,         bcmFieldQualifyL4DstPort,               WARP(bcm_field_qualify_L4DstPort),      STAGE_IFP},
    {QUAL::l4_src_port,         bcmFieldQualifyL4SrcPort,               WARP(bcm_field_qualify_L4SrcPort),      STAGE_IFP},
    {QUAL::ip_protocol,         bcmFieldQualifyIpProtocol,              WARP(bcm_field_qualify_IpProtocol),     STAGE_IFP},
    {QUAL::dscp,                bcmFieldQualifyDSCP,                    WARP(bcm_field_qualify_DSCP),           STAGE_IFP},
    {QUAL::ttl,                 bcmFieldQualifyTtl,                     WARP(bcm_field_qualify_Ttl),            STAGE_IFP},
    {QUAL::tcp_control,         bcmFieldQualifyTcpControl,              WARP(bcm_field_qualify_TcpControl),     STAGE_IFP},
    {QUAL::ext_header_type,     bcmFieldQualifyExtensionHeaderType,     WARP(bcm_field_qualify_ExtensionHeaderType),    STAGE_IFP},
    {QUAL::ext_header_sub_code, bcmFieldQualifyExtensionHeaderSubCode,  WARP(bcm_field_qualify_ExtensionHeaderSubCode), STAGE_IFP},
    {QUAL::ip6_flow_label,      bcmFieldQualifyIp6FlowLabel,            WARP(bcm_field_qualify_Ip6FlowLabel),   STAGE_IFP},
    {QUAL::dst_ip6,             bcmFieldQualifyDstIp6,                  WARP(bcm_field_qualify_DstIp6),         STAGE_VFP},
    {QUAL::src_ip6,             bcmFieldQualifySrcIp6,                  WARP(bcm_field_qualify_SrcIp6),         STAGE_VFP}
};

static
const FIELD_INFO * get_field_info(QUAL::PDF field)
{
    typedef map<QUAL::PDF, const FIELD_INFO *> DB_TYPE;
    static DB_TYPE db;

    if(0 == db.size()) {
        for(UINT8 i=0; i<(sizeof(field_info)/sizeof(FIELD_INFO)); i++){
            db.insert(make_pair(field_info[i].field, &field_info[i]));
        }
    }

    DB_TYPE::iterator it=db.find(field);
    ENSURE(it != db.end(), "unsupported qualification field");
    ASSERT(field == it->second->field);

    return it->second;
}

static
void set_qual(int unit, int eid, QUAL::PDF field, const QUAL::MATCH &match)
{
    const FIELD_INFO *info = get_field_info(field);
    ENSURE_OK(
        info->func(unit, eid, match));
}

void CIRCLE_FLOW::FP_WRAP::set_quals(int unit, int eid, const QUAL::PDF_RULES &rules)
{
    for(QUAL::PDF_RULES::const_iterator it=rules.begin();
            it!=rules.end(); it++) {
        set_qual(unit, eid, it->first, it->second);
    }
}

void CIRCLE_FLOW::FP_WRAP::set_qset(bcm_field_qset_t &qset, FP_STAGE stage)
{
    for(UINT8 i=0; i<(sizeof(field_info)/sizeof(FIELD_INFO)); i++){
        if(field_info[i].stage == stage) {
            BCM_FIELD_QSET_ADD(qset, field_info[i].qset);
        }
    }
}

QUAL::PDF_RULES CIRCLE_FLOW::FP_WRAP::split_rules(const QUAL::PDF_RULES &all, FP_STAGE stage)
{
    QUAL::PDF_RULES rules;

    for(QUAL::PDF_RULES::const_iterator it=all.begin(); it!=all.end(); it++) {
        const FIELD_INFO *info = get_field_info(it->first);
        if(info->stage == stage) {
            rules[it->first] = it->second;
        }
    }

    return rules;
}
