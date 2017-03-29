#include "utility/export/_dependency.h"
#include "utility/export/error.h"
using namespace UTILITY;

extern "C"{

    unsigned long err_printf(const unsigned long errortype,
                             const unsigned long errorclass,
                             const char *name,
                             const char *file,
                             const int line,
                             const char *format, ...);
}

static char module_name [] = "CFLO";

static
void error_printf (const char *str)
{
    err_printf(0, 0, module_name, "", 0, str);
}

static
void err_terminate(const char *file, int line, const char *str)
{
    err_printf(0, 2, module_name, file, line, str);
}

namespace SYS_ADAPT {
    void error_init(void)
    {
        error_set_output(error_printf);
        error_set_terminate(err_terminate);
    }
}
