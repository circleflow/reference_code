
#include "qual.h"
using namespace CIRCLE_FLOW::QUAL_HELPER;
#include "bytes_bits.h"
#include "field/field_parser.h"

#include "utility/export/error.h"

#include <string.h>

static
PDF_INFO pdf_info [] = {
        {"dst_mac",             QUAL::dst_mac,              8*6 },
        {"src_mac",             QUAL::src_mac,              8*6 },
        {"ether_type",          QUAL::ether_type,           8*2 },
        {"outer_tpid",          QUAL::outer_tpid,           8*2 },
        {"outer_vlan_cfi",      QUAL::outer_vlan_cfi,       1   },
        {"outer_vlan_pri",      QUAL::outer_vlan_pri,       3   },
        {"outer_vlan_id",       QUAL::outer_vlan_id,        12  },
        {"inner_tpid",          QUAL::inner_tpid,           8*2 },
        {"inner_vlan_cfi",      QUAL::inner_vlan_cfi,       1   },
        {"inner_vlan_pri",      QUAL::inner_vlan_pri,       3   },
        {"inner_vlan_id",       QUAL::inner_vlan_id,        12  },
        {"dst_ip",              QUAL::dst_ip,               8*4 },
        {"src_ip",              QUAL::src_ip,               8*4 },
        {"l4_dst_port",         QUAL::l4_dst_port,          8*2 },
        {"l4_src_port",         QUAL::l4_src_port,          8*2 },
        {"ip_protocol",         QUAL::ip_protocol,          8   },
        {"dscp",                QUAL::dscp,                 8   },
        {"ttl",                 QUAL::ttl,                  8   },
        {"tcp_control",         QUAL::tcp_control,          8   },
        {"ext_header_type",     QUAL::ext_header_type,      8   },
        {"ext_header_sub_code", QUAL::ext_header_sub_code,  8   },
        {"ip6_flow_label",      QUAL::ip6_flow_label,       8*4 },
        {"dst_ip6",             QUAL::dst_ip6,              8*16},
        {"src_ip6",             QUAL::src_ip6,              8*16}
};

static
const PDF_INFO &get_info(QUAL::PDF pdf)
{
    for(UINT8 i=0; i<(sizeof(pdf_info)/sizeof(PDF_INFO)); i++) {
        if(pdf_info[i].pdf == pdf) {
            return (pdf_info[i]);
        }
    }

    ERROR("unexpected pdf");
    return pdf_info[0];   //only to remove compile warning
}

static
const PDF_INFO &get_info(const char *field_name)
{
    for(UINT8 i=0; i<(sizeof(pdf_info)/sizeof(PDF_INFO)); i++) {
        if(0 == strcmp(pdf_info[i].name,field_name)) {
            return (pdf_info[i]);
        }
    }

    ERROR("unexpected pdf");
    return pdf_info[0];   //only to remove compile warning
}

static
BITS make_mask(int field_len, int prefix_len)
{
    ENSURE(field_len>0, "field length zero");

    if(prefix_len == FULL_MASK) {
        prefix_len = field_len;
    }

    ENSURE(prefix_len <= field_len, "prefix length exceed the field length");

    BITS bits(field_len, 0);

    for(int i=0; i<prefix_len; i++) {
        bits[i] = 1;
    }

    return bits;
}

QUAL::PDF_RULE QUAL_HELPER::make_pdf_rule(QUAL::PDF field,
                                          const BYTES &value,
                                          int prefix_len)
{
    EXP_RECAP_START;

    const PDF_INFO &info = get_info(field);

    QUAL::PDF_RULE rule;
    rule.first = field;

    BITS mask = make_mask(info.bit_len, prefix_len);
    rule.second.mask  = bits_to_bytes(mask);
    rule.second.value = value;

    size_fit(rule.second.value, info.bit_len);
    size_fit(rule.second.mask,  info.bit_len);

    rule.second.value &= rule.second.mask;

    return rule;

    EXP_RECAP_END;
}

QUAL::PDF_RULE QUAL_HELPER::make_pdf_rule(const string &field_name,
                                          const TEXT &value,
                                          int prefix_len)
{
    EXP_RECAP_START;

    const PDF_INFO &info = get_info(field_name.c_str());

    QUAL::PDF_RULE rule;
    rule.first = info.pdf;;

    HEX_PARSER parser;
    rule.second.value = parser.text_to_bytes(value);
    size_fit(rule.second.value, info.bit_len);

    BITS mask = make_mask(info.bit_len, prefix_len);
    rule.second.mask  = bits_to_bytes(mask);

    rule.second.value &= rule.second.mask;

    return rule;

    EXP_RECAP_END;
}

QUAL::PDF_RULE QUAL_HELPER::make_pdf_rule(const string &field_name,
                                          const TEXT &value,
                                          const TEXT &mask)
{
    EXP_RECAP_START;

    const PDF_INFO &info = get_info(field_name.c_str());
    
    QUAL::PDF_RULE rule;
    rule.first = info.pdf;

    HEX_PARSER parser;
    rule.second.value = parser.text_to_bytes(value);
    rule.second.mask  = parser.text_to_bytes(mask);
    size_fit(rule.second.value, info.bit_len);
    size_fit(rule.second.mask,  info.bit_len);
    
    rule.second.value &= rule.second.mask;

    return rule;

    EXP_RECAP_END;
}

QUAL::UDF_RULE QUAL_HELPER::make_udf_rule(const FIELD_BLOCK &pkt,
                                          const FIELD::INDEX &field,
                                          int prefix_len)
{
    EXP_RECAP_START;

    QUAL::UDF_RULE rule;

    const FIELD &f = pkt.field(field);

    int offset = pkt.offset_of_bit(field);
    int size = f.size_of_bit();

    //make the match window to be byte aligned
    BITS mask  = make_mask(size, prefix_len);
    BITS value = bytes_to_bits((BYTES) f);
    size_fit(value, size);

    int prefix_padding = 0;
    if(offset%8) {
        prefix_padding = offset%8;
        offset -= prefix_padding;
        size += prefix_padding;
    }

    int suffix_padding = 0;
    if(size%8) {
        suffix_padding = 8 - (size%8);
        size += suffix_padding;
    }

    ENSURE(0 == (offset%8), "invalid offset, must be byte line up");
    ENSURE(0 == (size%8), "invalid size, must be byte line up");

    rule.first.offset = offset/8;
    rule.first.size   = size/8;

    if(prefix_padding) {
        mask.insert (mask.begin(),  prefix_padding, 0);
        value.insert(value.begin(), prefix_padding, 0);
    }

    if(suffix_padding) {
        mask.insert (mask.end(),  suffix_padding, 0);
        value.insert(value.end(), suffix_padding, 0);
    }

    rule.second.mask  = bits_to_bytes(mask);
    rule.second.value = bits_to_bytes(value);

    rule.second.value &= rule.second.mask;

    return rule;

    EXP_RECAP_END;

}

QUAL::UDF_RULE QUAL_HELPER::make_udf_rule(int offset,
                                          int size,
                                          const TEXT &value,
                                          const TEXT &mask)
{
    EXP_RECAP_START;

    QUAL::UDF_RULE rule;

    rule.first.offset = offset;
    rule.first.size = size;

    HEX_PARSER parser;
    rule.second.value = parser.text_to_bytes(string(value));
    rule.second.mask = parser.text_to_bytes(string(mask));
    size_fit(rule.second.value, size*8);
    size_fit(rule.second.mask, size*8);

    rule.second.value &= rule.second.mask;

    return rule;

    EXP_RECAP_END;
}

QUAL::UDF_RULE QUAL_HELPER::make_udf_rule(int offset,
                                          int size,
                                          const TEXT &value,
                                          int prefix_len)
{
    EXP_RECAP_START;

    QUAL::UDF_RULE rule;

    rule.first.offset = offset;
    rule.first.size = size;

    HEX_PARSER parser;
    rule.second.value = parser.text_to_bytes(string(value));
    size_fit(rule.second.value, size*8);

    BITS mask = make_mask(8*size, prefix_len);
    rule.second.mask = bits_to_bytes(mask);

    rule.second.value &= rule.second.mask;

    return rule;

    EXP_RECAP_END;
}

PDF_INFO QUAL_HELPER::get_pdf_info(QUAL::PDF pdf)
{
    return get_info(pdf);
}
