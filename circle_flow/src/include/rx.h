
#ifndef RX_H_
#define RX_H_

#include "type_ext.h"
#include "qual.h"
#include "cpu.h"

class RX{
public:

    RX(const QUAL &qual);
    ~RX();

    COUNTER counter(bool clear);

    void snoop_start(const CPU::RX::PKT_CB &cb, UINT16 max_pkt);
    void snoop_stop(void);

private:

    RX(const RX & ref);
    RX & operator = (const RX & ref);

    class IMPL;
    IMPL *pimpl;
};

class RX_LATENCY{
public:

    RX_LATENCY(const PORT_NAME_SET &in_ports,
               const QUAL::UDF_RULE &udf_rule,
               const CPU::RX::PKT_CB &cb);
    ~RX_LATENCY();

private:

    RX_LATENCY(const RX_LATENCY & ref);
    RX_LATENCY & operator = (const RX_LATENCY & ref);

    class IMPL;
    IMPL *pimpl;
};

#endif /* RX_H_ */
