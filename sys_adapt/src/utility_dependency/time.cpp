#include "utility/export/base_type.h"
#include "utility/export/_dependency.h"
#include "utility/export/error.h"
using namespace UTILITY;

extern "C" {
    #include <sal/core/thread.h>

    unsigned long xtm_gettimestamp (unsigned long *seconds,
                                    unsigned long *microseconds);
}

static
UINT32 time_get_elapsed_ms()
{
    unsigned long seconds=0, microseconds=0;

    ENSURE_OK(xtm_gettimestamp(&seconds, &microseconds));

    return seconds*1000 + microseconds/1000;
}

static
UINT32 time_max_elapsed_ms()
{
    return 0xffffffff;
}

static
void time_sleep_ms(UINT32 ms)
{
    sal_usleep(ms*1000);
}

namespace SYS_ADAPT {
    void time_init(void)
    {
        time_set_op(time_get_elapsed_ms,
                    time_max_elapsed_ms,
                    time_sleep_ms);
    }
}
