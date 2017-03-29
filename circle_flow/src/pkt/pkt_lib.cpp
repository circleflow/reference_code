
#include "field_concrete.h"
#include "field_block.h"

#include "ethernet.h"
#include "ipv4.h"
#include "ipv6.h"

#include "pkt_def.h"
#include "bytes_bits.h"

using namespace CIRCLE_FLOW;

#include "utility/export/base_type.h"
#include "utility/export/error.h"

#include <vector>
using std::vector;

FIELD_BLOCK FB_ETH_II::make_ETH_II(int length)
{
    PKT_ETH_II_DEF;

    UINT32 head_len = fb.size_of_bit()/8;
    fb.field(FB_ETH_II::L3_PAYLOAD) = BYTES((length-head_len), 0);
    return fb;
}

FIELD_BLOCK FB_ETH_II::make_ETH_II( const FIELD_BLOCK &payload )
{
    PKT_ETH_II_DEF;

    fb.extend(FB_ETH_II::L3_PAYLOAD, payload);
    return fb;
}

FIELD_BLOCK FB_VLAN_TAG::make_VLAN_TAG(void)
{
    VLAN_TAG_DEF;

    return fb;
}

FIELD_BLOCK FB_FLOW_TRACK_TAG::make_FLOW_TRACK_TAG(void)
{
    FLOW_TRACK_DEF;

    return fb;
}

static BYTES cac_ihl (unsigned int size_of_bit)
{
    ENSURE(0 == (size_of_bit%32), "invalid length");

    unsigned int u32 = size_of_bit/32;
    ENSURE(u32<=15, "invalid length");

    return make_bytes((UINT8)u32);
}

#define ip_hdr_chksum  FIELD_CACULATOR::ip_hdr_chksum
#define len_of_byte_16 FIELD_CACULATOR::len_of_byte_16

FIELD_BLOCK FB_IPV4::make_IPV4(int length)
{
    PKT_IPV4_DEF;

    length = length-fb.size_of_bit()/8;
    ENSURE(length>=0, "invalid length");
    fb.field(FB_IPV4::L4_PAYLOAD) = BYTES(length, 0);

    return fb;
}

FIELD_BLOCK FB_IPV6::make_IPV6(int length)
{
    PKT_IPV6_DEF;

    length = length-fb.size_of_bit()/8;
    ENSURE(length>=0, "invalid length");
    fb.field(FB_IPV6::L4_PAYLOAD) = BYTES(length, 0);

    return fb;
}
