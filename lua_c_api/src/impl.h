
#ifndef SRC_PACKET_IMPL_H_
#define SRC_PACKET_IMPL_H_

#include <vector>
using std::vector;
#include <string>
using std::string;

#include "circle_flow/export/field/field_block.h"
using namespace CIRCLE_FLOW;

namespace LUA_C_CIRCLE_FLOW {

    FIELD_BLOCK & get_pkt(const string &pkt_name);

    RATE convert_rate(const char *rate_unit, int rate_val);

    ostream & operator << (ostream & os, const RATE &rate);
    ostream & operator << (ostream & os, const BURST &burst);

    ostream & operator << (ostream & os, const COUNTER &cnt);
    ostream & operator << (ostream & os, const PORT_NAME_SET &ports);
    ostream & operator << (ostream & os, const SNOOP_PKT &snoop);
}

using namespace LUA_C_CIRCLE_FLOW;

#endif /* SRC_PACKET_IMPL_H_ */
