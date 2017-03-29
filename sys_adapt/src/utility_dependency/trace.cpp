#include "utility/export/_dependency.h"
#include "utility/export/error.h"
using namespace UTILITY;

static unsigned long trc_id;
static char module_name [] = "CFLO";

extern "C" {
    unsigned long trc_register (char name [4],
                                unsigned long prio,
                                unsigned long *prid);

    int trc_setLevel (unsigned long prid,
                      unsigned long level);

    int trc_printf (unsigned long prid,
                    const char *fmt,
                    ...);
}

static
void trace_printf (const char *str)
{
    trc_printf(trc_id, str);
}

namespace SYS_ADAPT {

void trace_init(void)
{
    ENSURE(0 == trc_register( module_name, 200, &trc_id ));

    trc_setLevel(trc_id, 0);

    trace_set_output(trace_printf);

}

}
