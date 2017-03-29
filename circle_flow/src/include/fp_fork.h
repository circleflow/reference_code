
#ifndef FP_FORK_H_
#define FP_FORK_H_

#include "type_ext.h"
#include "cpu.h"
#include "utility/export/unique.h"

//duplicate pkt to both engine port and front port
class FP_FORK_PORT {
public:
    FP_FORK_PORT(UINT8 unit,UINT8 engine, UINT8 front);
    ~FP_FORK_PORT();

private:

    FP_FORK_PORT(const FP_FORK_PORT &ref);
    FP_FORK_PORT & operator = (const FP_FORK_PORT &ref);

    class IMPL;
    IMPL *pimpl;
};

/*duplicate pkt to front port and latency port, the latter then loopback to CPU with timestamp
 *why not just duplicated to CPU(timestamped) and front port, only because fp actions cannot co-exist.
 */
class FP_FORK_LATENCY {
public:
    FP_FORK_LATENCY(UINT8 unit, UINT8 engine, UINT8 front, UINT8 latency);
    ~FP_FORK_LATENCY(void);

private:
    FP_FORK_LATENCY(const FP_FORK_LATENCY &ref);
    FP_FORK_LATENCY & operator = (const FP_FORK_LATENCY &ref);

    class IMPL;
    IMPL *pimpl;
};

//direct latency pkts to cpu with timestamp
class FP_LATENCY_PORT {
public:
    FP_LATENCY_PORT(UINT8 _unit, UINT8 port, CPU::QID qid);
    ~FP_LATENCY_PORT();

private:
    FP_LATENCY_PORT (const FP_LATENCY_PORT &);
    FP_LATENCY_PORT & operator = (const FP_LATENCY_PORT &);

    class IMPL;
    IMPL *pimpl;
};


#endif /* FP_FORK_H_ */
