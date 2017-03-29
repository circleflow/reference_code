
#ifndef SDK_H_
#define SDK_H_

extern "C" {
#include "bcm/types.h"
}

#include "type_ext.h"

#include <vector>
using std::vector;

namespace CIRCLE_FLOW {
    //////////////// helper ///////////////
    bcm_pbmp_t pbmp_all(int unit);
    bcm_pbmp_t pbmp_e_all(int unit);

    bcm_pbmp_t pbmp(UINT8 port);
    bcm_pbmp_t pbmp(UINT8 port_1, UINT8 port_2);
    bcm_pbmp_t pbmp(const vector<UINT8> &ports);
}

#endif /* SDK_H_ */
