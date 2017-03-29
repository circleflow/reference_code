
#include "hal_port.h"
#include "hal.h"
#include "defs.h"
#include "hal.h"

extern "C" {
#include "bcm/port.h"
#include "bcm/link.h"
#include "sal/core/dpc.h"
#include "bcm/stat.h"
}


static
DS_SET ds_cvt(const bcm_port_abil_t bcm_ability)
{
    DS_SET ds_set;

#define DS_MAPPING(bcm_ds, api_ds) \
        if(bcm_ability & bcm_ds) ds_set.insert(api_ds);

    DS_MAPPING(BCM_PORT_ABIL_10MB_FD,   FD_10MB);
    DS_MAPPING(BCM_PORT_ABIL_10MB_HD,   HD_10MB);
    DS_MAPPING(BCM_PORT_ABIL_100MB_FD,  FD_100MB);
    DS_MAPPING(BCM_PORT_ABIL_100MB_HD,  HD_100MB);
    DS_MAPPING(BCM_PORT_ABIL_1000MB_FD, FD_1000MB);
    DS_MAPPING(BCM_PORT_ABIL_1000MB_HD, HD_1000MB);
    DS_MAPPING(BCM_PORT_ABIL_2500MB_FD, FD_2500MB);
    DS_MAPPING(BCM_PORT_ABIL_2500MB_HD, HD_2500MB);
    DS_MAPPING(BCM_PORT_ABIL_10GB_FD,   FD_10GB);
    DS_MAPPING(BCM_PORT_ABIL_10GB_HD,   HD_10GB);

#undef DS_MAPPING

    return ds_set;
}

static
bcm_port_abil_t ds_cvt(const DS_SET &ds_set)
{
    bcm_port_abil_t bcm_ability = 0;

#define DS_MAPPING(api_ds, bcm_ds) \
    if(ds_set.find(api_ds) != ds_set.end()) bcm_ability |= bcm_ds;

    DS_MAPPING(FD_10MB,   BCM_PORT_ABIL_10MB_FD);
    DS_MAPPING(HD_10MB,   BCM_PORT_ABIL_10MB_HD);
    DS_MAPPING(FD_100MB,  BCM_PORT_ABIL_100MB_FD);
    DS_MAPPING(HD_100MB,  BCM_PORT_ABIL_100MB_HD);
    DS_MAPPING(FD_1000MB, BCM_PORT_ABIL_1000MB_FD);
    DS_MAPPING(HD_1000MB, BCM_PORT_ABIL_1000MB_HD);
    DS_MAPPING(FD_2500MB, BCM_PORT_ABIL_2500MB_FD);
    DS_MAPPING(HD_2500MB, BCM_PORT_ABIL_2500MB_HD);
    DS_MAPPING(FD_10GB,   BCM_PORT_ABIL_10GB_FD);
    DS_MAPPING(HD_10GB,   BCM_PORT_ABIL_10GB_HD);

#undef DS_MAPPING

    return bcm_ability;
}

static
DUPLEX_SPEED ds_cvt(int speed, bool duplex)
{
#define DS_MAPPING(_speed,_duplex, ds) if(speed==_speed && duplex==_duplex) return ds

    DS_MAPPING(10,    true,  FD_10MB);
    DS_MAPPING(10,    false, HD_10MB);
    DS_MAPPING(100,   true,  FD_100MB);
    DS_MAPPING(100,   false, HD_100MB);
    DS_MAPPING(1000,  true,  FD_1000MB);
    DS_MAPPING(1000,  false, HD_1000MB);
    DS_MAPPING(2500,  true,  FD_2500MB);
    DS_MAPPING(2500,  false, HD_2500MB);
    DS_MAPPING(10000, true,  FD_10GB);
    DS_MAPPING(10000, false, HD_10GB);

#undef DS_MAPPING

    return DS_END;
}

ABILITY CIRCLE_FLOW::hal_port_ability_get(UINT8 unit, UINT8 port)
{
    ABILITY ability;
    bcm_port_abil_t  bcm_ability;
    ENSURE_OK(
        bcm_port_ability_get(unit, port, &bcm_ability));

    ability.ds_set = ds_cvt(bcm_ability);
    ability.auto_nego = bcm_ability & BCM_PORT_ABIL_AN ? true : false;
    ability.pause = bcm_ability & BCM_PORT_ABIL_PAUSE_RX ? true : false;

    return ability;
}

STATUS CIRCLE_FLOW::hal_port_status_get(UINT8 unit, UINT8 port)
{
    STATUS status;
    int duplex, speed, an, link, pause_rx, pause_tx;

    ENSURE_OK(
        bcm_port_duplex_get(unit, port, &duplex));

    ENSURE_OK(
        bcm_port_speed_get(unit, port, &speed));

    ENSURE_OK(
        bcm_port_autoneg_get(unit, port, &an));

    ENSURE_OK(
        bcm_port_link_status_get(unit, port, &link));

    ENSURE_OK(
        bcm_port_pause_get(unit, port, &pause_tx, &pause_rx));

    status.ds = ds_cvt(speed, duplex>0);
    status.an_enable = an;
    status.link = link;
    status.pause = pause_rx;

    return status;
}

static
int extract_speed(DUPLEX_SPEED ds)
{
    switch(ds)
    {
        case FD_10MB:
            return 10;
        case FD_100MB:
            return 100;
        case FD_1000MB:
            return 1000;
        case FD_2500MB:
            return 2500;
        case FD_10GB:
            return 10000;
        default:
            ERROR("invalid duplex_speed value");
            return 0;//only to remove compile warning
    }
}


void CIRCLE_FLOW::hal_port_auto_nego(UINT8 unit, UINT8 port, const ADVERT &advert)
{
    bcm_port_abil_t ability = ds_cvt(advert.ds_set);

    ability |= advert.pause ? BCM_PORT_ABIL_PAUSE_RX : 0;
    ENSURE_OK(
        bcm_port_advert_set(unit, port, ability));

    ENSURE_OK(
        bcm_port_autoneg_set(unit, port, 1));
}

void CIRCLE_FLOW::hal_port_forced(UINT8 unit, UINT8 port, const MODE &mode)
{
    AUTO_TRC_FUNC;

    TRC_VERBOSE("\r\n unit %d port %d duplex %d pause %d", unit, port, (int)mode.ds, (int)mode.pause);

    ENSURE_OK(
        bcm_port_autoneg_set(unit, port, 0));

    ENSURE_OK(
        bcm_port_speed_set(unit, port, extract_speed(mode.ds)));

    ENSURE_OK(
        bcm_port_duplex_set(unit, port, 1));

    ENSURE_OK(
        bcm_port_pause_set(unit, port, 0, mode.pause));
}

TRX_CNT CIRCLE_FLOW::hal_port_cnt_get (UINT8 unit, UINT8 port)
{
    TRX_CNT cnt;

    ENSURE_OK(
        bcm_stat_get(unit, port, snmpIfInOctets, &cnt.rx.byte));

    ENSURE_OK(
        bcm_stat_get(unit, port, snmpIfOutOctets, &cnt.tx.byte));

    ENSURE_OK(
        bcm_stat_get(unit, port, snmpDot1dTpPortInFrames, &cnt.rx.pkt));

    ENSURE_OK(
        bcm_stat_get(unit, port, snmpDot1dTpPortOutFrames, &cnt.tx.pkt));

    return cnt;
}

void CIRCLE_FLOW::hal_port_cnt_clear (UINT8 unit, UINT8 port)
{
    ENSURE_OK(
        bcm_stat_clear(unit, port));
}

static vector<HAL_LINK_CB> db_link_cb;

static
void link_cb_dispatch(void * _unit, void * _port, void * _link, void * , void *)
{
    UINT8 unit = reinterpret_cast<UINT32> (_unit);
    UINT8 port = reinterpret_cast<UINT32> (_port);
    bool link  = reinterpret_cast<UINT32> (_link);

    for(UINT8 i=0; i<db_link_cb.size(); i++) {
        db_link_cb[i](unit, port, link);
    }
}

static
void link_cb(int unit,
             bcm_port_t port,
             bcm_port_info_t *info)
{
    /* decouple to another thread, avoid mutex dead lock
     * when doing port setting from circle flow API, sdk will trigger link event call back
     * in the same thread, which would lead to dead lock*/
    ENSURE_OK(
        sal_dpc(link_cb_dispatch,(void*)unit, (void*)port, (void*)info->linkstatus, 0, 0));
}

void CIRCLE_FLOW::hal_port_link_callback_register(HAL_LINK_CB cb)
{
    static bool sdk_registered = false;

    if(! sdk_registered) {
        int unit;
        UNIT_ALL_ITER(unit) {
            ENSURE_OK(
                bcm_linkscan_register(unit, link_cb));
        }

        sdk_registered = true;
    }

    db_link_cb.push_back(cb);
}

void CIRCLE_FLOW::hal_port_max_frame_size_set(UINT32 size)
{
    UINT8 unit, port;

    UNIT_ALL_ITER(unit) {
        PORT_E_ALL_ITER(unit, port) {
            ENSURE_OK(
                bcm_port_frame_max_set(unit, port, size));
        }
    }
}

