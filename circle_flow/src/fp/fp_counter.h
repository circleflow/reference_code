
#ifndef FP_COUNTER_H_
#define FP_COUNTER_H_

#include "type_ext.h"

class FP_COUNTER {
public:
    FP_COUNTER(int unit, int gid);
    ~FP_COUNTER();

    void attach(int eid);
    COUNTER get(bool clear);

private:
    int unit, cnt_id;
};

#endif /* FP_COUNTER_H_ */
