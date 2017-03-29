
#ifndef FLOW_TRACK_H_
#define FLOW_TRACK_H_

#include "type_ext.h"
#include "qual.h"

class FLOW_TRACK {
public:

    FLOW_TRACK(void);
    ~FLOW_TRACK();

    static UINT8 size(void) ;   //in byte
    static UINT8 max_sn(void);

    BYTES flow_bytes() const;
    BYTES sn_bytes(UINT8 sn) const;

    QUAL::MATCH flow_match(void) const;
    QUAL::MATCH sn_match(void) const;

    UINT8 extract_sn(const BYTES &bytes) const;

private:
    FLOW_TRACK(const FLOW_TRACK &ref);
    FLOW_TRACK & operator = (const FLOW_TRACK &ref);

    class IMPL;
    IMPL *pimpl;

};


#endif /* FLOW_TRACK_H_ */
