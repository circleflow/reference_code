
#ifndef SRC_INCLUDE_FP_BURST_Q_H_
#define SRC_INCLUDE_FP_BURST_Q_H_

#include "type_ext.h"

//apply policer on the flow(include latency measurement pkt), for tx burst control
class FP_BURST_COSQ {
public:

    FP_BURST_COSQ(UINT8 unit, UINT8 engine, UINT8 cosq, UINT16 flow_vid);
    ~FP_BURST_COSQ();

    void burst (const BURST &burst);

private:

    FP_BURST_COSQ(const FP_BURST_COSQ &ref);
    FP_BURST_COSQ & operator = (const FP_BURST_COSQ &ref);

    class IMPL;
    IMPL *pimpl;

};


#endif /* SRC_INCLUDE_FP_BURST_Q_H_ */
