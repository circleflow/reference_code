
#include "defs.h"

#include <sstream>
using std::ostringstream;
using std::endl;
#include <algorithm>
using std::find;

string CIRCLE_FLOW::bytes_dump(const BYTES &bytes, UINT8 length)
{
    ostringstream oss;

    oss.setf (std::ios::hex , std::ios::basefield);
    oss.setf (std::ios::right , std::ios::adjustfield);
    oss.unsetf (std::ios::showbase);

    length = bytes.size()<length ? bytes.size() : length;

    UINT8 i=0;
    for(; i<(length-1); i++) {

        oss.width(2);
        oss.fill('0');

        oss<<(short)bytes[i];

        if(15 == (i%16)) {
            oss<<endl;
        } else {
            oss<<":";
        }
    }

    oss.width(2);
    oss.fill('0');

    oss<<(unsigned short)bytes[i];

    return oss.str();
}

TRACE_FILTER CIRCLE_FLOW::_trc_filter_func("cf_func");
TRACE_FILTER CIRCLE_FLOW::_trc_filter_verbose("cf_verbose");
TRACE_FILTER CIRCLE_FLOW::_trc_filter_latency("cf_latency");
TRACE_FILTER CIRCLE_FLOW::_trc_filter_packet("cf_packet");

