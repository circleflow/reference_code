
#ifndef QUAL_EXTRACT_H_
#define QUAL_EXTRACT_H_

#include "utility/export/smart_ptr_u.h"
#include "qual.h"
#include "field.h"
#include "udf.h"
#include "fp_rx.h"
#include "tpid.h"


namespace QUAL_EXTRACT {

    UINT8 extract_unit(const PORT_NAME_SET &ports);
    vector<UINT8> extract_ports(const PORT_NAME_SET &ports);

    //allocate UDF resource depend on UDF_QUAL
    class UDFS {
        vector< shared_ptr<UDF> > udfs;

    public:
        UDFS(UINT8 unit, const QUAL::UDF_RULES &udf_rules);
        QUAL::MATCH get_match(void);
    };

    //allocate TPID resource dpend on PDF TPID qualifier
    class TPIDS {
        vector< shared_ptr<OUTER_TPID> > outer_tpids;
        vector< shared_ptr<INNER_TPID> > inner_tpids;

    public:
        TPIDS(const PORT_NAME_SET &ports, const QUAL::PDF_RULES &pdf_rules);
    };
}

#endif /* QUAL_EXTRACT_H_ */
