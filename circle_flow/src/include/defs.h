
#ifndef SRC_INCLUDE_DEFS_H_
#define SRC_INCLUDE_DEFS_H_

#define SAFE_DELETE(ptr) \
        if(ptr){\
            delete ptr;\
            ptr = 0;\
        }

#include "utility/export/error.h"
using namespace UTILITY;

namespace CIRCLE_FLOW {
    namespace  DEVICE {
        bool is_init_done();
    }
}

#define ENSURE_INIT_OK  \
        if(!(CIRCLE_FLOW::DEVICE::is_init_done())) ERROR("init not completed:");

#include "utility/export/mutex_u.h"
struct CIRCLE_FLOW_API { };
#define AUTO_MUTEX_API AUTO_MUTEX<CIRCLE_FLOW_API> _auto_mutex

#include "utility/export/timer.h"
typedef TIMER_GUARD<AUTO_MUTEX<CIRCLE_FLOW_API>::lock, AUTO_MUTEX<CIRCLE_FLOW_API>::unlock> TIMER_SAFE;

#include "utility/export/trace.h"
using namespace UTILITY;

namespace CIRCLE_FLOW {
    extern TRACE_FILTER _trc_filter_func;
    extern TRACE_FILTER _trc_filter_verbose;
    extern TRACE_FILTER _trc_filter_latency;
    extern TRACE_FILTER _trc_filter_packet;
}

#define AUTO_TRC_FUNC DURATION_TRACE _auto_trc_func(_trc_filter_func,__FUNCTION__)

#define TRC_VERBOSE(args...) _TRACE(_trc_filter_verbose, args)
#define TRC_LATENCY(args...) _TRACE(_trc_filter_latency, args)
#define TRC_PACKET(args...)  _TRACE(_trc_filter_packet,  args)

#include "type_ext.h"
namespace CIRCLE_FLOW {
    string bytes_dump(const BYTES &bytes, UINT8 length);
}

#define ARRAY_NUM(array) (sizeof(array)/sizeof(array[0]))

#define RATE_CAC_INTERVAL 1000   //ms

#endif /* SRC_INCLUDE_DEFS_H_ */
