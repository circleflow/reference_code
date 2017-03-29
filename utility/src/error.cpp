#include "_dependency.h"
#include "error.h"
using namespace UTILITY;

EXP_ERROR::EXP_ERROR(const string &_info)
:info(_info) {}

const char *
EXP_ERROR::what() const throw ()
{
    return info.c_str();
}

void
EXP_ERROR::append(const string &_info)
{
    info += _info;
}

EXP_ERROR::~EXP_ERROR() throw() {}


static F_OUTPUT f_output=0;
void UTILITY::error_set_output(F_OUTPUT f)
{
    f_output = f;
}

#include "mutex_u.h"
struct ERR_PRINTF_T { };
#define AUTO_MUTEX_ERR_PRT AUTO_MUTEX<ERR_PRINTF_T> _auto_mutex;

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
void ERR::printf(const char *fmt, ...)
{
    static char buff[4096];

    AUTO_MUTEX_ERR_PRT;

    buff[0] = 0;

    va_list args;
    va_start (args, fmt);

    vsnprintf(buff, sizeof(buff), fmt, args);

    va_end (args);

    if(f_output) {
        f_output(buff);
    } else {
        std::cout<<buff;
    }
}

#include "log.h"

#define ERROR_ID_BASE (-1)
#define LOG_ID(err_id)   (ERROR_ID_BASE-err_id)
#define ERROR_ID(log_id) (ERROR_ID_BASE-log_id)

int ERR::log(const string &str)
{
    return ERROR_ID(LOG(str));
}

string ERR::get_log(int error_id)
{
    return LOG::get_log(LOG_ID(error_id));
}

struct STR_PRINTF_T { };
#define AUTO_MUTEX_STR_PRT AUTO_MUTEX<STR_PRINTF_T> _auto_mutex;

string ERR::_str_printf(void)
{
    static string str("an error occured");

    return str;
}

string ERR::_str_printf(const char *fmt, ...)
{
    static char buff[4096];

    AUTO_MUTEX_STR_PRT;

    buff[0] = 0;

    va_list args;
    va_start (args, fmt);

    vsnprintf(buff, sizeof(buff), fmt, args);

    va_end (args);

    return string(buff);
}

static F_TERMINATE f_terminate=0;
void UTILITY::error_set_terminate(F_TERMINATE f)
{
    f_terminate = f;
}

void ERR::terminate(const char *file, int line, const string &str)
{
    if(f_terminate) {
        f_terminate(file, line, str.c_str());
    } else {
        //no ERR_PRINTF, it may lead to recursive call, for example, if anything wrong in mutex
        ::printf("\r\n ERROR:%s:%d, %s", file, line, str.c_str());
        std::terminate();
    }
}
