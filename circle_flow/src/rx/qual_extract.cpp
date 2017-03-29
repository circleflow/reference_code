
#include "defs.h"
#include "port_impl.h"
#include "qual_extract.h"
#include "bytes_bits.h"

#include "utility/export/error.h"

using std::vector;
#include <algorithm>
using std::for_each;

using namespace QUAL_EXTRACT;

UINT8 QUAL_EXTRACT::extract_unit(const PORT_NAME_SET &ports)
{
    ENSURE(ports.size()>0, "empty port");
    ENSURE(is_same_unit(ports), "ports not on same unit");

    return PORT_IMPL::get_front(*(ports.begin())).unit;
}

vector<UINT8> QUAL_EXTRACT::extract_ports(const PORT_NAME_SET &ports)
{
    ENSURE(is_same_unit(ports),  "ports not on same unit");

    vector<UINT8> pids;

    for(PORT_NAME_SET::const_iterator it = ports.begin(); it!=ports.end(); it++) {
        PORT_ID port = PORT_IMPL::get_front(*it);
        pids.push_back(port.index);
    }

    return pids;
}

UDFS::UDFS(UINT8 unit, const QUAL::UDF_RULES &udf_rules)
{
    if(udf_rules.size() > 0) {

        for(QUAL::UDF_RULES::const_iterator it=udf_rules.begin();
                it!=udf_rules.end(); it++) {

            shared_ptr<UDF> ptr(new UDF(unit, it->first.offset, it->first.size));

            ptr->set_match(it->second);

            udfs.push_back(ptr);
        }
    }
}

QUAL::MATCH UDFS::get_match(void)
{
    QUAL::MATCH merged;

    for(vector< shared_ptr<UDF> >::iterator it=udfs.begin(); it!=udfs.end(); it++)
    {
        QUAL::MATCH single = (*it)->get_match();

        //each udf has no overlapped field
        ENSURE(BYTES(single.mask.size(), 0) == (merged.mask & single.mask),
                "overlapped mask");

        merged.mask  |= single.mask;
        merged.value |= single.value;
    }

    return merged;
}


class EXTRACT_TPID {
public:
    EXTRACT_TPID(QUAL::PDF _pdf):pdf(_pdf), tpid(0) {}

    void operator () (const QUAL::PDF_RULES::value_type &rule)
    {
        if(pdf == rule.first) {
            ENSURE(0==tpid, "duplicated TPID qualification");

            const QUAL::MATCH &match=rule.second;
            ENSURE(2==match.value.size(), "incorrect TPID size");

            tpid = (match.value[0]<<8)|match.value[1];
        }
    }

    QUAL::PDF pdf;
    UINT16 tpid;
};

TPIDS::TPIDS(const PORT_NAME_SET &ports, const QUAL::PDF_RULES &pdf_rules)
{
    {
        EXTRACT_TPID extract(QUAL::outer_tpid);
        extract = for_each(pdf_rules.begin(), pdf_rules.end(), extract);

        if(extract.tpid != 0 ) {
            for(PORT_NAME_SET::const_iterator port_it=ports.begin();
                    port_it!=ports.end();
                        port_it++) {
                shared_ptr<OUTER_TPID> ptr(new OUTER_TPID(PORT_IMPL::get_front(*port_it), extract.tpid));
                outer_tpids.push_back(ptr);
            }
        }
    }

    {
        EXTRACT_TPID extract(QUAL::inner_tpid);
        extract = for_each(pdf_rules.begin(), pdf_rules.end(), extract);

        if(extract.tpid != 0 ) {
            for(PORT_NAME_SET::const_iterator port_it=ports.begin();
                    port_it!=ports.end();
                        port_it++) {

                shared_ptr<INNER_TPID> ptr(new INNER_TPID(PORT_IMPL::get_front(*port_it), extract.tpid));
                inner_tpids.push_back(ptr);
            }
        }
    }
}




