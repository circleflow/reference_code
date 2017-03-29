
#ifndef FP_RX_H_
#define FP_RX_H_

#include "type_ext.h"
#include "qual.h"
#include "cpu.h"

struct FP_QUAL {
    UINT8 unit;
    vector<UINT8> in_ports;

    QUAL::PDF_RULES pdf_rules;
    QUAL::MATCH udf_match;

    UINT8 pri;

    FP_QUAL(UINT8 _unit, const vector<UINT8> &_in_ports, const QUAL::PDF_RULES &pdf, const QUAL::MATCH &udf, UINT8 _pri)
    :unit(_unit), in_ports(_in_ports), pdf_rules(pdf), udf_match(udf), pri(_pri)
    { }
};

class HAL_FP_RX {
public:
    HAL_FP_RX(const FP_QUAL &qual);
    ~HAL_FP_RX();

    void snoop_start(UINT8 max_pkt, CPU::QID qid);
    void snoop_stop(void);

    COUNTER counter(bool clear);

private:
    HAL_FP_RX(const HAL_FP_RX &);
    HAL_FP_RX & operator = (const HAL_FP_RX &);

    struct IMPL;
    IMPL *pimpl;
};

class HAL_FP_LATENCY_RX {
public:
    HAL_FP_LATENCY_RX(const FP_QUAL &qual, CPU::QID qid);
    ~HAL_FP_LATENCY_RX();

private:
    HAL_FP_LATENCY_RX(const HAL_FP_LATENCY_RX &);
    HAL_FP_LATENCY_RX & operator = (const HAL_FP_LATENCY_RX &);

    struct IMPL;
    IMPL *pimpl;
};


#endif /* FP_RX_H_ */
