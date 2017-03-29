
#include "defs.h"
#include "hal.h"
#include "sdk.h"
#include "utility/export/error.h"

extern "C" {
#include "soc/drv.h"
}


const HAL_UNIT_SPECS & CIRCLE_FLOW::hal_unit_specs(UINT8 unit)
{
    static HAL_UNIT_SPECS specs;
    static bool initialized = false;

    if(false == initialized) {
        specs.device_id = SOC_PCI_DEVICE(unit);
        specs.device_id <<= 16;
        specs.device_id |= SOC_PCI_REVISION(unit);

        specs.cpu_port_id = 0;

        initialized = true;
    }

    return specs;
}

const vector<UINT8> & CIRCLE_FLOW::hal_unit_all(void)
{
    static vector<UINT8> units;

    if(0 == units.size()) {

        ASSERT(soc_ndev>0);

        for(UINT8 unit=0; unit<soc_ndev; unit++) {
            units.push_back(unit);
        }
    }

    return units;
}

const vector<int> & CIRCLE_FLOW::hal_speed_all(UINT8 unit, UINT8 port)
{
    static vector<int> speeds_ge;
    static vector<int> speeds_xe;

    if(0 == speeds_ge.size()) {
        speeds_ge.push_back(10);
        speeds_ge.push_back(100);
        speeds_ge.push_back(1000);
    }

    if(0 == speeds_xe.size()) {
        speeds_xe.push_back(10);
        speeds_xe.push_back(100);
        speeds_xe.push_back(1000);
        speeds_xe.push_back(2500);
        speeds_xe.push_back(10000);
    }

    if(IS_GE_PORT(unit,port)) {
        return speeds_ge;
    } else if(IS_XE_PORT(unit, port)) {
        return speeds_xe;
    } else {
        ERROR("unknown port type!");

        static vector<int> speeds_null;
        return speeds_null;
    }
}

const vector<UINT8> & CIRCLE_FLOW::hal_port_e_all(UINT8 unit)
{
    static vector<UINT8> ports;

    if(0 == ports.size()) {

        bcm_pbmp_t pbm = pbmp_e_all(unit);

        UINT8 port;
        PBMP_ITER(pbm, port) {
            ports.push_back(port);
        }

    }

    return ports;
}
