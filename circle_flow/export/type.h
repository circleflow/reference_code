
#ifndef CIRCLE_FLOW_TYPE_H_
#define CIRCLE_FLOW_TYPE_H_

#include <string>
using std::string;
#include <set>
using std::set;
#include <vector>
using std::vector;

namespace CIRCLE_FLOW {


    typedef string PORT_NAME;
    typedef set<std::string> PORT_NAME_SET;


    struct RATE {
        enum TYPE{
            //include preemble and inter-gap
            KBITS_PER_SECOND,
            MBITS_PER_SECOND,
            GBITS_PER_SECOND
        };

        TYPE type;
        unsigned int value;

        RATE(TYPE _type=RATE::KBITS_PER_SECOND, unsigned int _value=0) : type(_type), value(_value) {}
    };

    struct BURST {
        enum TYPE {
            PKT
        };

        TYPE type;
        unsigned int value;

        BURST(TYPE _type=BURST::PKT, unsigned int _value=0) : type(_type), value(_value) {}
    };

    struct COUNTER{
        unsigned long long pkt;
        unsigned long long byte;

        COUNTER(unsigned long long _pkt=0, unsigned long long _byte=0) : pkt(_pkt), byte(_byte) {}

        COUNTER   operator -  (const COUNTER &) const;
        COUNTER & operator -= (const COUNTER &);
        COUNTER   operator +  (const COUNTER &) const;
        COUNTER & operator += (const COUNTER &);
        COUNTER   operator /  (float);
        bool operator == (const COUNTER &) const;
    };

    struct TRX_CNT {
        COUNTER tx;
        COUNTER rx;

        TRX_CNT   operator -  (const TRX_CNT &) const;
        TRX_CNT & operator -= (const TRX_CNT &);
        TRX_CNT   operator +  (const TRX_CNT &) const;
        TRX_CNT & operator += (const TRX_CNT &);
        TRX_CNT   operator /  (float);

        bool operator == (const TRX_CNT &) const;
    };

    typedef string TEXT;                 //field/pkt value in string
    typedef vector<unsigned char> BYTES; //field/pkt value in digits

    typedef BYTES PACKET;
    typedef vector<PACKET> PACKETS;

    typedef unsigned int TIME_STAMP;

    struct SNOOP_PKT {
        TIME_STAMP time_stamp;  //us
        PACKET pkt;
    };

    typedef vector<SNOOP_PKT> SNOOP_PKTS;

}


#endif /* CIRCLE_FLOW_TYPE_H_ */
