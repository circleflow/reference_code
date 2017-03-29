
#ifndef PKTS_HELPER_H_
#define PKTS_HELPER_H_

#include "type_ext.h"

#include "utility/export/error.h"

#define TPID_HI 0x81
#define TPID_LO 0x00

static
void insert_vtag(const PACKET &src, PACKET &dst, UINT16 vid)
{
    if(&dst != &src) {
        dst = src;
    }

    UINT8 tag[] = {TPID_HI,TPID_LO,0x00,0x00};
    tag[2] = (vid>>8);
    tag[3] = (vid&0xff);

    vector<UINT8>::iterator pos = dst.begin() + 12;

    dst.insert(pos, &tag[0], &tag[4]);

}

static
void insert_vtag(const PACKETS &src, PACKETS &dst, UINT16 vid)
{
    if(&src != &dst) {
        dst.resize(src.size());
    }

    for(UINT8 i=0; i<src.size(); i++) {
        insert_vtag(src[i], dst[i], vid);
    }
}

static
UINT16 extract_ovid(const PACKET &pkt)
{
    ENSURE(pkt.size()>16, "invalid pkt size");
    ENSURE(TPID_HI == pkt[12], "invalid TPID value");
    ENSURE(TPID_LO == pkt[13], "invalid TPID value");

    UINT16 vid = (pkt[14]&0x0f)<<8;
    vid |= pkt[15];

    return vid;
}

static
vector<UINT16> pkts_len(const PACKETS &pkts)
{
    vector<UINT16> len;

    for(UINT8 i=0; i<pkts.size(); i++) {
        len.push_back (pkts[i].size());
    }

    return len;
}

static
void remove_otag(const PACKET &src, PACKET &dst)
{
    PACKET::const_iterator it = src.begin();
    dst.assign(it, it+12);
    dst.insert(dst.end(), it+16, src.end());
}

#endif /* PKTS_HELPER_H_ */
