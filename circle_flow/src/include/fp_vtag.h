
#ifndef SRC_INCLUDE_CLASSIFY_VTAG_H_
#define SRC_INCLUDE_CLASSIFY_VTAG_H_

#include "type_ext.h"

//vlan id used for classify flow with burst
class FP_BRUST_Q_VTAG {
public:
    FP_BRUST_Q_VTAG(UINT8 cosq);

    UINT16 get_vid();
    static UINT16 get_vid_mask();

private:
    FP_BRUST_Q_VTAG(const FP_BRUST_Q_VTAG& ref);
    FP_BRUST_Q_VTAG & operator = (const FP_BRUST_Q_VTAG& ref);

    UINT16 vid;
};

//vlan id used for classify latency flow, required by latency observer dispatch
class FP_LATENCY_VTAG {
public:
    FP_LATENCY_VTAG(UINT8 unit, UINT8 cosq);
    ~FP_LATENCY_VTAG();

    UINT16 get_vid();

    static UINT16 get_fp_data();
    static UINT16 get_fp_mask();

private:
    FP_LATENCY_VTAG(const FP_LATENCY_VTAG& ref);
    FP_LATENCY_VTAG & operator = (const FP_LATENCY_VTAG &ref);

    class IMPL;
    IMPL *pimpl;
};

#include <vector>
using std::vector;

//shared mapping between vid and cosq
class FP_SHARED_COSQ_VTAG {
public:
    struct MAPPING{
        UINT16 vid;
        UINT16 mask;
        UINT8  cosq;
    };

    static vector<MAPPING> get_mapping();
};

#endif /* SRC_INCLUDE_CLASSIFY_VTAG_H_ */
