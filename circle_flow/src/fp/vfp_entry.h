
#ifndef SRC_FP_VFP_ENTRY_H_
#define SRC_FP_VFP_ENTRY_H_

#include "type_ext.h"
#include "qual.h"
#include "vfp_rx.h"

class VFP_ENTRY {
    class IMPL;
    IMPL *pimpl;

public:
    VFP_ENTRY(UINT8 _unit, UINT8 port,
              VFP_CLASS_ID class_id,
              const QUAL::PDF_RULES &rules,
              UINT8 pri);
    ~VFP_ENTRY();
};


#endif /* SRC_FP_VFP_ENTRY_H_ */
