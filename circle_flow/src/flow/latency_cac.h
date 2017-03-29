
#include "utility/export/smart_ptr_u.h"
#include "utility/export/function_bind.h"
#include "type_ext.h"


class LATENCY_CAC {
public:
    LATENCY_CAC();
    ~LATENCY_CAC();

    typedef function<void (const vector<UINT8> & sn)> PKT_GEN;
    void start(const PKT_GEN &pkt_gen);
    void stop();

#define LATENCY_INVALID  -1
    int get_latency(void);

    void set_tx_timestamp(UINT8 sn, UINT32 timestamp);
    void set_rx_timestamp(UINT8 sn, UINT32 timestamp);

private:
    LATENCY_CAC(const LATENCY_CAC &);
    LATENCY_CAC & operator = (const LATENCY_CAC &);

    class IMPL;
    IMPL *pimpl;

};


