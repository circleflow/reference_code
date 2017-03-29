
#include "sdk.h"

extern "C" {
#include "soc/drv.h"
}

bcm_pbmp_t CIRCLE_FLOW::pbmp_all(int unit)
{
    return PBMP_ALL(unit);
}

bcm_pbmp_t CIRCLE_FLOW::pbmp_e_all(int unit)
{
    return PBMP_E_ALL(unit);
}

bcm_pbmp_t CIRCLE_FLOW::pbmp(UINT8 port)
{
    bcm_pbmp_t pbm;
    BCM_PBMP_CLEAR(pbm);

    BCM_PBMP_PORT_ADD(pbm, port);

    return pbm;
}

bcm_pbmp_t CIRCLE_FLOW::pbmp(UINT8 port_1, UINT8 port_2)
{
    bcm_pbmp_t pbm;
    BCM_PBMP_CLEAR(pbm);

    BCM_PBMP_PORT_ADD(pbm, port_1);
    BCM_PBMP_PORT_ADD(pbm, port_2);

    return pbm;
}

bcm_pbmp_t CIRCLE_FLOW::pbmp(const vector<UINT8> &ports)
{
    bcm_pbmp_t pbm;
    BCM_PBMP_CLEAR(pbm);

    for(UINT8 i=0; i<ports.size(); i++) {
        BCM_PBMP_PORT_ADD(pbm, ports[i]);
    }

    return pbm;
}


