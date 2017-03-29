
#include "bytes_bits.h"
#include "utility/export/error.h"

BYTES CIRCLE_FLOW::make_bytes(UINT8 data)
{
    BYTES v;
    v.push_back(data);

    return v;
}

BYTES CIRCLE_FLOW::make_bytes(UINT16 data)
{
    BYTES v;
    v.push_back((UINT8) (data>>8));
    v.push_back((UINT8) (data>>0));

    return v;
}

BYTES CIRCLE_FLOW::make_bytes(UINT32 data)
{
    BYTES v;
    v.push_back((UINT8) (data>>24));
    v.push_back((UINT8) (data>>16));
    v.push_back((UINT8) (data>>8));
    v.push_back((UINT8) (data>>0));

    return v;
}

BYTES_CONVERT::BYTES_CONVERT(const BYTES &_bytes)
: bytes(_bytes),str(0) {}

static UINT32 bytes_to_u32(const BYTES &bytes)
{
    UINT32 u32=0;
    UINT8 max=bytes.size()>4 ? 4 : bytes.size();

    BYTES::const_reverse_iterator it=bytes.rbegin();
    for(UINT8 i=0; i<max; i++) {
        u32 |= (*it)<<(i*8);
        it++;
    }

    return u32;
}

BYTES_CONVERT::operator UINT32 (void) const
{
    return bytes_to_u32(bytes);
}

BYTES_CONVERT::operator int (void) const
{
    return (int) bytes_to_u32(bytes);
}

BYTES_CONVERT::operator UINT16 (void) const
{
    return (UINT16) bytes_to_u32(bytes);
}

BYTES_CONVERT::operator UINT8 (void) const
{
    return (UINT8) bytes_to_u32(bytes);
}

BYTES_CONVERT::operator UINT8* (void)
{
    if(0 == str) {
        str = new UINT8[bytes.size()];
        for(UINT32 i=0; i<bytes.size(); i++) {
            str[i] = bytes[i];
        }
    }

    return str;
}

BYTES_CONVERT::~BYTES_CONVERT()
{
    if(str) delete str;
}

BYTES CIRCLE_FLOW::operator &  (const BYTES &lhs, const BYTES &rhs)
{
    if((0==lhs.size()) && (0==rhs.size())) {
        return BYTES();
    }

    if(0 == lhs.size()) {
        return BYTES(rhs.size(), 0);
    }

    if(0 == rhs.size()) {
        return BYTES(lhs.size(), 0);
    }

    ENSURE(lhs.size() == rhs.size(), "unmatched length");

    BYTES bytes;
    UINT32 max = lhs.size();
    for(UINT32 i=0; i<max; i++) {
        bytes.push_back(lhs[i] & rhs[i]);
    }

    return bytes;
}

void  CIRCLE_FLOW::operator &= (BYTES &lhs, const BYTES &rhs)
{
    lhs = lhs & rhs;
}

BYTES CIRCLE_FLOW::operator |  (const BYTES &lhs, const BYTES &rhs)
{
    if((0==lhs.size()) && (0==rhs.size())) {
        return BYTES();
    }

    if(0 == lhs.size()) {
        return rhs;
    }

    if(0 == rhs.size()) {
        return lhs;
    }

    ENSURE(lhs.size() == rhs.size(), "unmatched length");

    BYTES bytes;
    UINT32 max = lhs.size();
    for(UINT32 i=0; i<max; i++) {
        bytes.push_back(lhs[i] | rhs[i]);
    }

    return bytes;
}

void  CIRCLE_FLOW::operator |= (BYTES &lhs, const BYTES &rhs)
{
    lhs = lhs | rhs;
}

/*
 network transfer data in order of:
    byte of low address transfer first than byte of high address
 for example: to transfer "11:22:33",
    byte "11" will be put into wire prior than byte "22"
 btw, network transfer data actually is not bit by bit, but quad by quad, or oct by oct, depend on signal code pattern.
 type "BYTES" is the presentation from perspective of network data.
 type "BITS" is used for fields catenate (if more than one fields construct a byte).

 for example, the last 2 bytes of vlan tag is consisted by 3 fields: PBIT[3b]DEI[1b]VID[12b]
 when convert into bytes:
 byte_0: PBIT[bit2-bit0],CFI[bit0],VID[bit11-bit8]
 byte_1: VID[bit7-bit0]
 pls note, when part of VID bits merged into the byte_0, the MSB bits is selected.
 to convenient bit concatenate, the bit sequence in BITS is ordered by MSB -> LSB,
 for example, turn a byte of BYTES into BITS, the bit sequence in BITS is: bit7,bit6...bit0
 in this case, when concatenate 2 fields, just simply concatenate the bits of second field after first field.
 */

BITS CIRCLE_FLOW::bytes_to_bits(const BYTES &bytes)
{
    BITS bits;

    bits.reserve(bytes.size()*8);

    for(BYTES::const_iterator it=bytes.begin(); it!=bytes.end(); it++) {
        //MSB first
        bits.push_back(((*it)>>7) & 0x1);
        bits.push_back(((*it)>>6) & 0x1);
        bits.push_back(((*it)>>5) & 0x1);
        bits.push_back(((*it)>>4) & 0x1);
        bits.push_back(((*it)>>3) & 0x1);
        bits.push_back(((*it)>>2) & 0x1);
        bits.push_back(((*it)>>1) & 0x1);
        bits.push_back(((*it)>>0) & 0x1);
    }

    return bits;
}

BYTES CIRCLE_FLOW::bits_to_bytes(const BITS &bits)
{
    BYTES bytes;
    UINT8 byte = 0;
    UINT8 bit_idx = 0;

    for(BITS::const_reverse_iterator it=bits.rbegin(); it!=bits.rend(); it++) {

        UINT8 bit = (*it) ? 1 : 0;
        byte |= (bit<<bit_idx);

        if(7 == bit_idx) {
            bytes.insert(bytes.begin(), byte);
            byte = 0;
        }

        bit_idx = (bit_idx+1)%8;
    }

    if(0 != bit_idx) {
        bytes.insert(bytes.begin(), byte);
    }

    return bytes;
}

void CIRCLE_FLOW::size_fit(BITS &bits, UINT32 size_of_bit)
{
    int size = bits.size() - size_of_bit;
    if(size > 0) {
        bits.erase(bits.begin(), bits.begin()+size);
    } else if(size<0) {
        bits.insert(bits.begin(), (-size), 0);
    }
}

void CIRCLE_FLOW::size_fit(BYTES &bytes, UINT32 size_of_bit)
{
    BITS bits = bytes_to_bits(bytes);
    size_fit(bits, size_of_bit);
    bytes = bits_to_bytes(bits);
}

BYTES CIRCLE_FLOW::pattern_fit(const BYTES &pattern, UINT32 size_of_byte)
{
    BYTES output;
    BYTES::const_iterator it=pattern.begin();

    for(UINT32 i=0; i<size_of_byte; i++) {
        output.push_back(*it);

        it++;
        if(it == pattern.end()) {
            it=pattern.begin();
        }
    }

    return output;
}
