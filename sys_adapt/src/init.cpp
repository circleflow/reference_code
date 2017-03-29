#include <api.h>

#include "utility/export/error.h"
#include "utility/export/base_type.h"
#include "utility/export/trace.h"
using namespace UTILITY;

namespace SYS_ADAPT {

    void trace_init(void);
    void error_init(void);
    void cli_init(void);
    void mutex_init(void);
    void thread_init(void);
    void time_init(void);

    void device_init(void);
}

using namespace SYS_ADAPT;

//root init entrance
extern "C" {

int sys_adapt_init(void)
{
    EXP_FREE_START;

    mutex_init();
    thread_init();
    time_init();

    trace_init();
    error_init();
    cli_init();

    device_init();

    EXP_FREE_END;

    return 0;
}

}
