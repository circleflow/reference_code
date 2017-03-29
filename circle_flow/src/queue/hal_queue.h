
#ifndef QUEUE_HAL_H_
#define QUEUE_HAL_H_

#include "type_ext.h"

namespace CIRCLE_FLOW {

    void hal_q_create_shaper(UINT8 unit, UINT8 port, UINT8 cosq,
                             UINT32 min_kbps, UINT32 max_kbps);

    struct TOKEN_REFRESH{
        UINT32 min;
        UINT32 max;

        TOKEN_REFRESH(UINT32 min=0, UINT32 max=0) : min(min), max(max) {}
    };

    TOKEN_REFRESH hal_q_get_shaper(UINT8 unit, UINT8 port, UINT8 cosq);
    void          hal_q_set_shaper(UINT8 unit, UINT8 port, UINT8 cosq, TOKEN_REFRESH refresh);

    COUNTER hal_q_get_counter(UINT8 unit, UINT8 port, UINT8 qid, bool clear);

    UINT8 hal_q_num(UINT8 unit, UINT8 port_id);

    bool hal_q_is_empty(UINT8 unit, UINT8 port, UINT8 qid);
}

#endif /* QUEUE_HAL_H_ */
