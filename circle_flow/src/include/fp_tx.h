
#ifndef SRC_INCLUDE_FP_TX_H_
#define SRC_INCLUDE_FP_TX_H_

#include "type_ext.h"

class HAL_FP_TX_STOP {
public:
    HAL_FP_TX_STOP(UINT8 unit, UINT8 engine, UINT16 ovid);
    ~HAL_FP_TX_STOP();

private:
    HAL_FP_TX_STOP(const HAL_FP_TX_STOP &);
    HAL_FP_TX_STOP & operator = (const HAL_FP_TX_STOP &);

    struct IMPL;
    IMPL *pimpl;
};


#endif /* SRC_INCLUDE_FP_TX_H_ */
