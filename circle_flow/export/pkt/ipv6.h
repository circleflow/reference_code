
#ifndef CIRCLE_FLOW_PKT_IPV6_H_
#define CIRCLE_FLOW_PKT_IPV6_H_

#include "circle_flow/export/field/field_concrete.h"

namespace CIRCLE_FLOW {

    namespace FB_IPV6 {
        static const string IPV6       = "ipv6";
        static const string VERSION    = "version";
        static const string TC         = "traffic_class";
        static const string FLB        = "flow_label";
        static const string PLEN       = "payload_lenth";
        static const string NH         = "next_header";
        static const string HLMT       = "hop_limit";
        static const string SRC_IP     = "source_ip";
        static const string DST_IP     = "destination_ip";
        static const string L4_PAYLOAD = "l4_payload";

        #define PKT_IPV6_DEF \
            BLOCK_START(IPV6) \
              FIXED_SIZE(VERSION,    4, "06") \
              FIXED_SIZE(TC,         8, "00") \
              FIXED_SIZE(FLB,       20, "00-00-00") \
              CACULATOR (PLEN,     2*8, L4_PAYLOAD, CAC_WHITE_LIST, len_of_byte_16) \
              FIXED_SIZE(NH,         8, "00") \
              FIXED_SIZE(HLMT,       8, "00") \
              FIXED_SIZE(SRC_IP,  16*8, "00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00") \
              FIXED_SIZE(DST_IP,  16*8, "00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00") \
              RESIZABLE (L4_PAYLOAD, 0) \
            BLOCK_END

        FIELD_BLOCK make_IPV6(int length=40);

    }
}

#endif /* EXPORT_PKT_IPV6_H_ */
