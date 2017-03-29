
#ifndef CPU_H_
#define CPU_H_

#include "type_ext.h"
#include "queue.h"

#include "utility/export/function_bind.h"

namespace CIRCLE_FLOW {
    namespace CPU{

        typedef UINT8 QID;

        class RX{
        public:
            RX (UINT8 unit, const vector<UINT8> &ports);
            ~RX();

            //pkt
            typedef function<void (UINT8 unit, const PACKET &pkt, UINT32 time_stamp)> PKT_CB;
            void bind(const PKT_CB &cb);
            void unbind();

            //info
            UINT8 get_unit(void) const { return unit; }
            const vector<UINT8> & get_ports(void) const { return ports; }
            QID get_qid(void) const { return qid; }

        private:
            UINT8 unit;
            vector<UINT8> ports;
            QID qid;
        };

        void tx(const QUEUE &queue, const PACKETS &pkts);
    }
}


#endif /* CPU_H_ */
