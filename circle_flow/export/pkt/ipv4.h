
#ifndef CIRCLE_FLOW_PKT_IPV4_H_
#define CIRCLE_FLOW_PKT_IPV4_H_

#include "circle_flow/export/field/field_concrete.h"

namespace CIRCLE_FLOW {

    namespace FB_IPV4 {
        static const string IPV4       = "ipv4";
        static const string VERSION    = "version";
        static const string IHL        = "ihl";
        static const string TOS        = "service_type";
        static const string TOT_LEN    = "total_length";
        static const string IP_ID      = "identification";
        static const string FLAGS      = "flags";
        static const string FRAG_OFF   = "fragment_offset";
        static const string TTL        = "time_to_live";
        static const string PROTOCOL   = "protocol";
        static const string CHK_SUM    = "header_checksum";
        static const string SRC_IP     = "source_ip";
        static const string DST_IP     = "destination_ip";
        static const string OPTIONS    = "options";
        static const string PADDING    = "padding";
        static const string L4_PAYLOAD = "l4_payload";

        #define PKT_IPV4_DEF \
            BLOCK_START(IPV4) \
              FIXED_SIZE(VERSION,    4, "04") \
              CACULATOR (IHL,        4, L4_PAYLOAD, CAC_BLACK_LIST, cac_ihl) \
              FIXED_SIZE(TOS,        8, "00") \
              CACULATOR (TOT_LEN,  2*8, "", CAC_BLACK_LIST, len_of_byte_16) \
              FIXED_SIZE(IP_ID,    2*8, "00-00") \
              FIXED_SIZE(FLAGS,      3, "00") \
              FIXED_SIZE(FRAG_OFF,  13, "00-00") \
              FIXED_SIZE(TTL,        8, "ff") \
              FIXED_SIZE(PROTOCOL,   8, "00") \
              CACULATOR( CHK_SUM,  2*8, L4_PAYLOAD, CAC_BLACK_LIST, ip_hdr_chksum) \
              FIXED_SIZE(SRC_IP,   4*8, "00-00-00-00") \
              FIXED_SIZE(DST_IP,   4*8, "00-00-00-00") \
              RESIZABLE( OPTIONS,    0) \
              RESIZABLE( PADDING,    0) \
              RESIZABLE( L4_PAYLOAD, 0) \
            BLOCK_END

        FIELD_BLOCK make_IPV4(int length=20);
    }

}

#endif /* EXPORT_PKT_IPV4_H_ */
