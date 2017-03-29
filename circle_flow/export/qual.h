
#ifndef CIRCLE_FLOW_QUAL_H_
#define CIRCLE_FLOW_QUAL_H_

#include "type.h"
#include "field/field_block.h"
using CIRCLE_FLOW::FIELD_BLOCK;

#include <vector>
using std::vector;
#include <map>
using std::map;
#include <utility>
using std::pair;
using std::make_pair;

namespace CIRCLE_FLOW {

    struct QUAL {

        //pre-defined field
        enum PDF {
            dst_mac,
            src_mac,
            ether_type,
            outer_tpid,
            outer_vlan_cfi,
            outer_vlan_pri,
            outer_vlan_id,
            inner_tpid,
            inner_vlan_cfi,
            inner_vlan_pri,
            inner_vlan_id,
            dst_ip,
            src_ip,
            l4_dst_port,
            l4_src_port,
            ip_protocol,
            dscp,
            ttl,
            tcp_control,
            ext_header_type,
            ext_header_sub_code,
            ip6_flow_label,
            dst_ip6,
            src_ip6,
            pdf_end
        };

        struct MATCH{
            BYTES   value;
            BYTES   mask;

            bool operator < (const MATCH &ref) const
            {
                return make_pair(value, mask) < make_pair(ref.value, ref.mask);
            }

            bool operator == (const MATCH &ref) const
            {
                return make_pair(value, mask) == make_pair (ref.value, ref.mask);
            }
        };

        typedef pair<PDF, MATCH> PDF_RULE;
        typedef map<PDF, MATCH>  PDF_RULES;

        //user defined field
        struct UDF {
            unsigned char offset;  //in byte
            unsigned char size;    //in byte

            bool operator < (const UDF &ref) const
            {
                return make_pair(offset, size) < make_pair(ref.offset, ref.size);
            }

            bool operator == (const UDF &ref) const
            {
                return make_pair(offset, size) == make_pair(ref.offset, ref.size);
            }
        };

        typedef pair<UDF, MATCH> UDF_RULE;
        typedef map<UDF, MATCH> UDF_RULES;

        PDF_RULES pdfs;
        UDF_RULES udfs;
        PORT_NAME_SET in_ports;

        enum{
            MIN_HIT_PRI=0,
            MAX_HIT_PRI=127
        };
        unsigned char hit_pri;

        QUAL():hit_pri(MIN_HIT_PRI) { }
    };

    //help function for generate qual rule
    namespace QUAL_HELPER {

        const int FULL_MASK = -1;

        QUAL::PDF_RULE make_pdf_rule(QUAL::PDF field,
                                     const BYTES &value,
                                     int bit_len_of_prefix_mask = FULL_MASK);

        QUAL::PDF_RULE make_pdf_rule(const string &field_name,
                                     const TEXT &value,
                                     int bit_len_of_prefix_mask = FULL_MASK);

        QUAL::PDF_RULE make_pdf_rule(const string &field_name,
                                     const TEXT &value,
                                     const TEXT &mask);

        QUAL::UDF_RULE make_udf_rule(const FIELD_BLOCK &,
                                     const FIELD::INDEX &,
                                     int bit_len_of_prefix_mask = FULL_MASK);

        QUAL::UDF_RULE make_udf_rule(int offset,
                                     int size,
                                     const TEXT &value,
                                     int bit_len_of_prefix_mask = FULL_MASK);

        QUAL::UDF_RULE make_udf_rule(int offset,
                                     int size,
                                     const TEXT &value,
                                     const TEXT &mask);

        struct PDF_INFO {
            const char * name;
            QUAL::PDF pdf;
            int bit_len;
        };

        PDF_INFO get_pdf_info(QUAL::PDF field);

    }
}

#endif /* QUAL_H_ */
