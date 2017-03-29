#include "fp_vtag.h"

#include "utility/export/unique.h"
#include "utility/export/error.h"

/* to classify flow, an internal purpose vlan tag is added to each flow
 * to carry the cosq for egress queue,
 * and to distinguish latency pkt flow when CPU received from latency loopback port.
 * this tag will be stripped at egress of front port.
 * to compensate the 4 byte of tag,
 * inter packet gap(IPG) is reduced 4 bytes accordingly, so still line rate assured.
 * it is unable to insert more than 1 tag, because the minimal IPG has reached.
 *
 * vlan id layout :
    bit: 11   10    9            4            0
          | 1b | 1b |     5b     |     5b     |

    bit 11  : flag, 1:tx flow, 0: latency flow
    bit 10  : not used
    bit 5~9 : latency flow id (combined with cosq, for cpu rx dispatch, find more at TX_LATENCY_VID)
    bit 0~4 : cosq

 * note, vlan id 0 should be avoided
 *       IFP unable to qualify zero VID, it has been tagged by PVID at ingress.
 * to carry more, the pbit of tag could be considered.
*/

#define NUM_COSQ_VID     32
#define NUM_LATENCY_VID  32

#define VID_BURST_Q_BASE 0x800
#define VID_LATENCY_BASE 0x000

#define VID_LATENCY_MASK      0x800
#define VID_BURST_Q_MASK      0xfff
#define VID_COSQ_Q_MASK       0x01f

#define VID_FORK_BURST_Q(cosq)      (VID_BURST_Q_BASE+cosq)
#define VID_FORK_LATENCY(flow,cosq) (VID_LATENCY_BASE+(flow<<5)+cosq)

FP_BRUST_Q_VTAG::FP_BRUST_Q_VTAG(UINT8 cosq)
{
    ASSERT(cosq<NUM_COSQ_VID);

    vid = VID_FORK_BURST_Q(cosq);
}

UINT16 FP_BRUST_Q_VTAG::get_vid()
{
    return vid;
}

UINT16 FP_BRUST_Q_VTAG::get_vid_mask()
{
    return VID_BURST_Q_MASK;
}

vector<FP_SHARED_COSQ_VTAG::MAPPING> FP_SHARED_COSQ_VTAG::get_mapping()
{
    vector<MAPPING> container;

    for(UINT16 cosq=0; cosq<NUM_COSQ_VID; cosq++) {
        MAPPING element;
        element.cosq = cosq;
        element.vid  = cosq;
        element.mask = VID_COSQ_Q_MASK;

        container.push_back(element);
    }

    return container;
}

struct FP_LATENCY_VTAG::IMPL {

    //start from 1, avoid zero VID
    typedef UNIQUE<FP_LATENCY_VTAG, UINT16, 1, (NUM_LATENCY_VID-1)> FLOW_ID;

    FLOW_ID flow;
    UINT16 vid;

    IMPL(UINT8 unit, UINT8 cosq)
    //by pool, increase number latency flow to NUM_LATENCY_VID*NUM_COSQ_VID per unit
    : flow(unit*NUM_COSQ_VID + cosq),
      vid(VID_FORK_LATENCY(flow.get_id(),cosq))
    { }
};


FP_LATENCY_VTAG::FP_LATENCY_VTAG(UINT8 unit, UINT8 cosq)
{
    pimpl = new IMPL(unit, cosq);
}

UINT16 FP_LATENCY_VTAG::get_vid()
{
    return pimpl->vid;
}

UINT16 FP_LATENCY_VTAG::get_fp_data()
{
    return VID_LATENCY_BASE;
}

UINT16 FP_LATENCY_VTAG::get_fp_mask()
{
    return VID_LATENCY_MASK;
}

FP_LATENCY_VTAG::~FP_LATENCY_VTAG()
{
    delete pimpl;
}
