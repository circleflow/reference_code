
#include "hal_tx.h"
#include "hal.h"

#include "utility/export/error.h"

extern "C" {
#include "bcm/port.h"
#include "bcm/vlan.h"
#include "bcm/cosq.h"
#include "bcm/switch.h"
}

static
void set_egress_rm_otag(UINT8 unit, UINT8 port, bool enable)
{
    bcm_vlan_action_set_t   action;
    bcm_vlan_action_set_t_init(&action);

    if(enable) {
        action.dt_outer = bcmVlanActionDelete;
        action.ot_outer = bcmVlanActionDelete;
    }

    ENSURE_OK(
        bcm_vlan_port_egress_default_action_set(unit, port, &action));
}

static
void set_inter_packet_gap(UINT8 unit, UINT8 port, UINT8 gap_of_bits)
{
    int speed;
    SPEED_ALL_ITER(unit, port,speed)
    {
        ENSURE_OK(
            bcm_port_ifg_set(unit, port, speed, BCM_PORT_DUPLEX_FULL, gap_of_bits));
    }
}

static
void set_scheduler(UINT8 unit, UINT8 port)
{
    bcm_pbmp_t pbmp;
    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_PORT_ADD(pbmp,port);

    ENSURE_OK(
        bcm_cosq_port_sched_set(unit, pbmp, BCM_COSQ_ROUND_ROBIN, (int *)0, 0));
}

void CIRCLE_FLOW::hal_init_front_port(UINT8 unit, UINT8 port)
{
    ENSURE_OK(
        bcm_port_learn_set(unit, port, BCM_PORT_LEARN_FWD));

    //disable ingress/egress vlan membership checking
    ENSURE_OK(
        bcm_port_vlan_member_set(unit, port, 0));

    ENSURE_OK(
        bcm_vlan_control_port_set(unit, port, bcmVlanTranslateIngressEnable,0));
    ENSURE_OK(
        bcm_vlan_control_port_set(unit, port, bcmVlanTranslateEgressEnable, 0));

    set_egress_rm_otag(unit, port, true);

    ENSURE_OK(
        bcm_port_enable_set(unit, port, 1));

    ENSURE_OK(
        bcm_port_loopback_set(unit, port, BCM_PORT_LOOPBACK_NONE));

    set_inter_packet_gap(unit, port, 12*8);

    ENSURE_OK(
        bcm_switch_control_port_set(unit, port, bcmSwitchMeterAdjust, 0));

    set_scheduler(unit, port);
}

void CIRCLE_FLOW::hal_init_engine_port(UINT8 unit, UINT8 port)
{
    ENSURE_OK(
        bcm_port_learn_set(unit, port, BCM_PORT_LEARN_FWD));

    //disable ingress/egress vlan membership checking
    ENSURE_OK(
        bcm_port_vlan_member_set(unit, port, 0));

    ENSURE_OK(
        bcm_vlan_control_port_set(unit, port, bcmVlanTranslateIngressEnable,0));
    ENSURE_OK(
        bcm_vlan_control_port_set(unit, port, bcmVlanTranslateEgressEnable, 0));

    set_egress_rm_otag(unit, port, false);

    ENSURE_OK(
        bcm_port_enable_set(unit, port, 1));

    ENSURE_OK(
        bcm_port_loopback_set(unit, port, BCM_PORT_LOOPBACK_MAC));

    //compensate the extra tag
    set_inter_packet_gap(unit, port, 8*8);

    ENSURE_OK(
        bcm_switch_control_port_set(unit, port, bcmSwitchMeterAdjust, 16));

    set_scheduler(unit, port);

}


void CIRCLE_FLOW::hal_init_latency_port(UINT8 unit, UINT8 port)
{
    //port init: no learning, loopback
    ENSURE_OK(
        bcm_port_learn_set(unit, port, BCM_PORT_LEARN_FWD));

    ENSURE_OK(
        bcm_port_vlan_member_set(unit, port, 0));

    ENSURE_OK(
        bcm_port_enable_set(unit, port, 1));

    ENSURE_OK(
        bcm_port_loopback_set(unit, port, BCM_PORT_LOOPBACK_MAC));

}

