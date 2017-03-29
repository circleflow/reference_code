
#ifndef CPU_HAL_H_
#define CPU_HAL_H_

#include "type.h"
#include "queue.h"

namespace CIRCLE_FLOW {

    typedef void (*hal_cpu_rx_cb)(UINT8 unit, UINT8 cosq, UINT8 src_port,
                                  const PACKET &pkt, UINT32 time_stamp);

    void hal_cpu_init(hal_cpu_rx_cb cb);
    void hal_cpu_tx(const QUEUE &q, const PACKETS &pkts);

}

#endif /* CPU_HAL_H_ */
