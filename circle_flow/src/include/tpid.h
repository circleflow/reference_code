
#ifndef TPID_H_
#define TPID_H_

#include "type_ext.h"

class OUTER_TPID {
public:
    OUTER_TPID(const PORT_ID &port, UINT16 tpid);
    ~OUTER_TPID();

private:
    OUTER_TPID();
    OUTER_TPID(const OUTER_TPID&ref);
    OUTER_TPID & operator = (const OUTER_TPID &ref);

    PORT_ID port;
    UINT16 tpid;
};

class INNER_TPID {
public:
    INNER_TPID(const PORT_ID &port, UINT16 tpid);
};


#endif /* TPID_H_ */
