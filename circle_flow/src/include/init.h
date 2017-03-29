
#ifndef SRC_INCLUDE_INIT_H_
#define SRC_INCLUDE_INIT_H_

#include "device.h"

namespace CIRCLE_FLOW {

    void fp_init();

    void queue_init();

    void cpu_init();

    void port_init(const vector<DEVICE::CONFIG::PAIR_PROFILE> & pair_profile,
                   const vector<DEVICE::CONFIG::LATENCY_PROFILE> & latency_profile,
                   UINT32 max_frame_size);

    void mmu_init();

    void tx_init(const vector<DEVICE::CONFIG::LATENCY_PROFILE> &profiles);

    void tpid_init();
}



#endif /* SRC_INCLUDE_INIT_H_ */
