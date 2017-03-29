
#ifndef MMU_H_
#define MMU_H_

#include "type_ext.h"

/* MMU is simply used for audit if there's enough buffer for TX_FLOW
 * without enough free buffer, pkts would be failed pending in the engine queue.
 * */

class MMU {
public:

    MMU(UINT8 unit, const vector<UINT16> &pkts_len);
    ~MMU();
private:
    UINT8  unit;
    UINT16 cells;

    MMU(const MMU &ref);
    MMU & operator = (const MMU &ref);
};



#endif /* MMU_H_ */
