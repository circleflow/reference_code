
#ifndef RESOURCE_H_
#define RESOURCE_H_


/*
 FP resource layout

 SW: single wide, 256 entry per slice
 DW: double wide, 128 entry per slice
 DS: dual slice

VFP:
++++++++
|      |
|  RX  |
| (DW) |
|      |
++++++++

IFP:
++++++++++++++++++++++++++
|        |       |       |
|   RX   |  FORK | BURST |
| (DW+DS)|  (DW) |  (SW) |
|        |       |       |
++++++++++++++++++++++++++

RX group:
 VFP and IFP are combined together to work as a single group for RX
 to contain all pre-defined and user-defined fields in one group.
 otherwise, we have to predict all kind of fields combination and create individual group for them
 create multiple RX group actually waste FP resource, and we only have very limited slices on hand.
 by combine VFP and IFP, it just need to create one RX group, for all fields, simple and predictable.

FORK group:
> flow stop entry
> fork flow to engine and front port
> fork latency flow to front and latency loopback port
> qualify latency rx pkt, copy to CPU with timestamp
  note, must be higher priority than RX snoop, because they have same action.
  it is done by group priority.
> qualify pkt receiving by latency loopback port, copty to CPU with timestamp

BURST group:
> default vid to cosq mapping
> specific flow with burst control, cosq mapping as well as

*/

//group ID
#define GID_RX_VFP      0
#define GID_RX_IFP      1
#define GID_FORK        2
#define GID_BURST_Q     3

//group priority
#define PRI_G_RX_VFP      0
#define PRI_G_RX_IFP      1
#define PRI_G_FORK        2
#define PRI_G_BURST_Q     3


//entry priority for GID_FORK group
#define PRI_E_FORK_PORT      0
#define PRI_E_FLOW_STOP      1
#define PRI_E_FORK_LATENCY   1

#define PRI_E_LATENCY_RX     0
#define PRI_E_LATENCY_PORT   0

//entry priority for GID_BURST_Q group
#define PRI_E_DEFAULT_Q   0
#define PRI_E_BURST_Q     1


#endif /* RESOURCE_H_ */
