
#ifndef SRC_FP_FP_RX_SDK_WRAP_H_
#define SRC_FP_FP_RX_SDK_WRAP_H_

#include "qual.h"
#include "type_ext.h"
extern "C" {
#include "bcm/field.h"
}

namespace CIRCLE_FLOW {
    namespace FP_WRAP {

        enum FP_STAGE {
            STAGE_VFP = 0,
            STAGE_IFP = 1
        };

        void set_quals(int unit, int eid, const QUAL::PDF_RULES &rules);

        void set_qset(bcm_field_qset_t &qset, FP_STAGE stage);

        QUAL::PDF_RULES split_rules(const QUAL::PDF_RULES &all, FP_STAGE stage);
    }
}

using namespace CIRCLE_FLOW::FP_WRAP;

#endif /* SRC_FP_FP_RX_SDK_WRAP_H_ */
