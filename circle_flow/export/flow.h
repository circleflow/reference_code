
#ifndef CIRCLE_FLOW_FLOW_H_
#define CIRCLE_FLOW_FLOW_H_

#include "type.h"
#include "port.h"
#include "qual.h"

namespace CIRCLE_FLOW {

    class TX_FLOW {
    public:
        TX_FLOW();
        ~TX_FLOW();

        void set(const PORT_NAME &tx_port);
        void set(const PACKETS &pkts);
        void set(const RATE &rate, const BURST &burst);

        void start(void);
        void stop(void);

        COUNTER counter(bool clear = false);
        COUNTER counter_rate();

        struct INFO {
            PORT_NAME port;
            RATE rate;
            BURST burst;
            bool is_started;
        };

        INFO info(void) const;

    private:
        struct IMPL;
        IMPL *pimpl;
    };

    class RX_FLOW {
    public:
        RX_FLOW();
        ~RX_FLOW();

        void set(const QUAL &qual);

        void start();
        void stop();

        void snoop(unsigned short max_pkt);   // 0 to stop snoop
        void dump(SNOOP_PKTS &copy);

        COUNTER counter(bool clear = false);
        COUNTER counter_rate();

        struct INFO {
            QUAL qual;
            bool is_started;
            bool is_snooping;
        };
        INFO info() const;

    private:
        struct IMPL;
        IMPL *pimpl;
    };

    /*
     * TRX_FLOW defines both TX and RX direction,
     * specially contains a unique flow tag with flow ID and serial number,
     * so it could be identified, and then measure out the latency time between TX and RX.
     */

    class TRX_FLOW {
    public:
        TRX_FLOW();
        ~TRX_FLOW();

        void set_tx(const PORT_NAME &tx_port);
        void set_tx(const FIELD_BLOCK &tx_pkt, const FIELD::INDEX &tx_flow_track);
        void set_tx(const RATE &tx_rate, const BURST &burst);

        void set_rx(const PORT_NAME_SET &rx_ports);
        void set_rx(unsigned int offset_of_rx_track);  //in unit of byte

        //tx and rx
        void trx_start();
        void trx_stop(void);

        //latency measurement
        void lm_start(void);
        void lm_stop(void);
        int  lm_get();  // in unit of us

        void snoop(unsigned short max_pkt);
        void dump(SNOOP_PKTS &);

        TRX_CNT counter(bool clear = false);
        TRX_CNT counter_rate();

        struct INFO {
            PORT_NAME tx_port;
            FIELD::NAME tx_pkt;
            RATE tx_rate;
            BURST tx_burst;
            PORT_NAME_SET rx_ports;
            unsigned int rx_track_offset;
            bool is_trx_started;
            bool is_snooping;
            bool is_lm_started;
        };
        INFO info() const;

    private:
        struct IMPL;
        IMPL *pimpl;
    };

}

#endif /* FLOW_H_ */
