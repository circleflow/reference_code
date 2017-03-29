

#ifndef TX_H_
#define TX_H_

#include "utility/export/smart_ptr_u.h"
#include "device.h"
#include "type_ext.h"
#include "cpu.h"

#include <string>
using std::string;

class TX_LATENCY;

class TX{
public:
    TX(const PORT_NAME &port);
    ~TX();

    void start(const PACKETS &pkts, RATE rate, BURST burst);
    void stop(void);

    COUNTER counter(bool clear);

    friend class TX_LATENCY;
private:

    TX(const TX &ref);
    TX & operator = (const TX &ref);

    struct IMPL;
    IMPL *pimpl;

};

class TX_LATENCY{
public:

    TX_LATENCY(shared_ptr<TX> tx, const CPU::RX::PKT_CB &cb);
    ~TX_LATENCY();

    void insert(const PACKETS & pkts);

private:
    TX_LATENCY(const TX_LATENCY &ref);
    TX_LATENCY & operator = (const TX_LATENCY &ref);

    struct IMPL;
    IMPL *pimpl;
};

#endif /* TX_H_ */
