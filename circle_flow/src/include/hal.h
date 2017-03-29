
#ifndef HAL_H_
#define HAL_H_

#include "type_ext.h"


//////////////////////// function //////////////////////////
namespace CIRCLE_FLOW {

    const vector<UINT8> & hal_unit_all(void);

    #define DEFAULT_UNIT 0

    typedef struct {
        UINT32 device_id;
        UINT8  cpu_port_id;
    } HAL_UNIT_SPECS;

    const HAL_UNIT_SPECS & hal_unit_specs(UINT8 unit=hal_unit_all()[0]);

    //////////////////////// helper /////////////////////////////////
    #define CPU_PORT_ID   (hal_unit_specs().cpu_port_id)

    #define UNIT_ALL_ITER(unit) \
        unit = hal_unit_all()[0];\
        for(UINT8 i=0; i<hal_unit_all().size(); \
                (++i)<hal_unit_all().size() ? unit=hal_unit_all()[i] : 0)

    const vector<int> & hal_speed_all(UINT8 unit, UINT8 port);

    #define SPEED_ALL_ITER(unit, port, speed) \
        speed=hal_speed_all(unit,port)[0];\
        for(UINT8 i=0; i<hal_speed_all(unit,port).size(); \
                (++i)<hal_speed_all(unit,port).size() ? speed=hal_speed_all(unit,port)[i] : 0)


    const vector<UINT8> & hal_port_e_all(UINT8 unit);   //return all ethernet ports, exclude cpu port

    #define PORT_E_ALL_ITER(unit, port) \
            port=hal_port_e_all(unit)[0];\
        for(UINT8 i=0; i<hal_port_e_all(unit).size(); \
                (++i)<hal_port_e_all(unit).size() ? port=hal_port_e_all(unit)[i] : 0)

}

#endif /* HAL_H_ */
