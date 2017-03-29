
#ifndef CIRCLE_FLOW_DEVICE_H_
#define CIRCLE_FLOW_DEVICE_H_

#include "type.h"
#include "port_type.h"

namespace CIRCLE_FLOW {

    namespace DEVICE{

        struct CONFIG{
            struct PAIR_PROFILE{

                struct PORT_PAIR {

                    CIRCLE_FLOW::PORT::INTERFACE phy_itf;
                    unsigned char engine;
                    unsigned char front;

                    bool operator == (const PORT_PAIR & ref) const
                    {
                        return phy_itf==ref.phy_itf
                               && engine==ref.engine
                               && front==ref.front;
                    }
                };

                PORT_NAME  name;
                unsigned char unit;
                vector<PORT_PAIR> port_pair;

                bool operator == (const PAIR_PROFILE & ref) const
                {
                    return  name==ref.name
                            && unit==ref.unit
                            && port_pair==ref.port_pair;
                }

            };

            //list of engine/front port pair
            vector<PAIR_PROFILE> pair_profile;

            struct LATENCY_PROFILE {
                unsigned char unit;
                unsigned char port;
            };

            //dedicated loopback port for latency measuring
            vector<LATENCY_PROFILE> latency_profile;

            unsigned int max_frame_size;
        };

        //device init interface
        void init(const CONFIG &);

        bool is_init_done();

    }
}

#endif /* DEVICE_H_ */
