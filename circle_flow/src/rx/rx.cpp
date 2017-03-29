
#include "rx.h"
#include "cpu.h"
#include "qual_extract.h"
using namespace QUAL_EXTRACT;

#include "utility/export/error.h"
#include "utility/export/smart_ptr_u.h"

#include <algorithm>
using std::copy;

#include <iterator>
using std::back_inserter;

class RX::IMPL {
public:

    UINT8 unit;
    vector<UINT8> ports;
    UDFS udfs;
    TPIDS tpids;
    shared_ptr<CPU::RX> cpu_rx;
    HAL_FP_RX fp;

    IMPL(UINT8 _unit, vector<UINT8> _ports, const QUAL &qual)
    : unit(_unit),
      ports(_ports),
      udfs(unit, qual.udfs),
      tpids(qual.in_ports, qual.pdfs),
      fp(FP_QUAL(unit, ports, qual.pdfs, udfs.get_match(), qual.hit_pri))
    { }

    void snoop_start(const CPU::RX::PKT_CB cb, UINT16 max_pkt)
    {
        cpu_rx.reset(new CPU::RX(unit, ports));
        cpu_rx->bind(cb);
        fp.snoop_start(max_pkt, cpu_rx->get_qid());
    }

    void snoop_stop()
    {
        fp.snoop_stop();
        cpu_rx.reset(); //unbind is not necessary since cpu_rx was destructed
    }
};

RX::RX(const QUAL &qual)
{
    pimpl = new IMPL(extract_unit(qual.in_ports),
                     extract_ports(qual.in_ports),
                     qual);
}

RX::~RX()
{
    delete pimpl;
}

COUNTER RX::counter(bool clear)
{
    return pimpl->fp.counter(clear);
}

void RX::snoop_start(const CPU::RX::PKT_CB &cb, UINT16 max_pkt)
{
    pimpl->snoop_start(cb, max_pkt);
}

void RX::snoop_stop(void)
{
    pimpl->snoop_stop();
}


class RX_LATENCY::IMPL {
    UDFS udfs;
    CPU::RX cpu_rx;
    HAL_FP_LATENCY_RX fp;
public:
    IMPL(UINT8 unit, vector<UINT8> ports,
         const QUAL::UDF_RULES &udf_rules,
         const CPU::RX::PKT_CB &cb)
    : udfs(unit, udf_rules),
      cpu_rx(unit, ports),
      fp(FP_QUAL(unit, ports, QUAL::PDF_RULES(), udfs.get_match(), 0),
      cpu_rx.get_qid())
    {
        cpu_rx.bind(cb);
    }
};

RX_LATENCY::RX_LATENCY(const PORT_NAME_SET &in_ports,
                       const QUAL::UDF_RULE &udf_rule,
                       const CPU::RX::PKT_CB &cb)
{
    QUAL::UDF_RULES rules;
    rules.insert(udf_rule);

    pimpl = new IMPL(extract_unit(in_ports),
                     extract_ports(in_ports),
                     rules, cb);
}

RX_LATENCY::~RX_LATENCY()
{
    delete pimpl;
}

