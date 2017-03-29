
#ifndef SRC_FP_VFP_RX_H_
#define SRC_FP_VFP_RX_H_

#include "qual.h"
#include "type_ext.h"

typedef UINT16 VFP_CLASS_ID;
#define NULL_VFP_CLASS_ID 0xffff

#define CLASS_DEST(class_id)    (class_id&0x3f)
#define CLASS_SOURCE(class_id)  ((class_id>>6)&0x3f)

class VFP_RX {
public:
    VFP_RX(UINT8 unit, const vector<UINT8> &ports, const QUAL::PDF_RULES &rules, UINT8 pri);
    ~VFP_RX();

    VFP_CLASS_ID get_class_id(void) { return class_id; }

private:
    UINT8 unit;
    vector<UINT8> ports;
    QUAL::PDF_RULES rules;
    VFP_CLASS_ID class_id;
};


#endif /* SRC_FP_VFP_RX_H_ */
