
#ifndef CIRCLE_FLOW_PKT_ETHERNET_H_
#define CIRCLE_FLOW_PKT_ETHERNET_H_

#include "circle_flow/export/field/field_concrete.h"

//////////////////// helper macro //////////////////

#define CAC_BLACK_LIST FIELD_CACULATOR::BLACK_LIST
#define CAC_WHITE_LIST FIELD_CACULATOR::WHITE_LIST

///////////////////////////////////////////////////

namespace CIRCLE_FLOW {

    namespace FB_ETH_II {

        static const string ETH_II     = "ethernet_II";
        static const string DST_MAC    = "dst_mac";
        static const string SRC_MAC    = "src_mac";
        static const string ETH_TYPE   = "ethernet_type";
        static const string L3_PAYLOAD = "l3_payload";
        static const string FCS        = "frame_checksum";

        #define PKT_ETH_II_DEF \
            BLOCK_START(ETH_II) \
              FIXED_SIZE( DST_MAC,    6*8, "00-00-00-00-00-00") \
              FIXED_SIZE( SRC_MAC,    6*8, "00-00-00-00-00-00") \
              FIXED_SIZE( ETH_TYPE,   2*8,             "00-00") \
              RESIZABLE ( L3_PAYLOAD, 0) \
              FIXED_SIZE( FCS,        4*8,       "00-00-00-00") \
            BLOCK_END

        /* the FCS field will always be caculated and overwrited by hw before putting on wire.
         * frame with wrong FCS also will be dropped in hw layer after receiving from wire.
         * so, in brief, sw don't perform any CRC algorithm on it.
         */

        FIELD_BLOCK make_ETH_II( int length=64 );
        FIELD_BLOCK make_ETH_II( const FIELD_BLOCK &payload );
    }

    namespace FB_VLAN_TAG {

        static const string VLAN_TAG = "vlan_tag";
        static const string TPID     = "tpid";
        static const string PBIT     = "pbit";
        static const string DEI      = "dei";
        static const string VLAN_ID  = "vlan_id";

        #define VLAN_TAG_DEF \
            BLOCK_START(VLAN_TAG) \
              FIXED_SIZE(TPID,   2*8, "81-00") \
              FIXED_SIZE(PBIT,     3,     "0") \
              FIXED_SIZE(DEI,      1,     "0") \
              FIXED_SIZE(VLAN_ID, 12,     "0") \
            BLOCK_END

        FIELD_BLOCK make_VLAN_TAG(void);
    }

    namespace FB_FLOW_TRACK_TAG {
        static const string  FLOW_TRACK_TAG = "flow_track_tag";
        static const string  FLOW_ID        = "flow_id";
        static const string  LATENCY_FLAG   = "latency_flag";
        static const string  LATENCY_SN     = "latency_sn";

        #define FLOW_TRACK_DEF  \
            BLOCK_START(FLOW_TRACK_TAG) \
              FIXED_SIZE(FLOW_ID,        28, "00") \
              FIXED_SIZE(LATENCY_FLAG,    1, "00") \
              FIXED_SIZE(LATENCY_SN,      3, "00") \
            BLOCK_END

        FIELD_BLOCK make_FLOW_TRACK_TAG(void);
    }


}

#endif
