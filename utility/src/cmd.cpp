
#include "cmd.h"
#include "_dependency.h"
using namespace UTILITY;
using namespace UTILITY::CMD;

#include "error.h"

#include <list>
using std::list;

struct INSTANCE {
    string key;
    HANDLER handler;
    HELPER helper;

    INSTANCE(const string &_key, const HANDLER &_handler, const HELPER &_helper)
    :key(_key),handler(_handler),helper(_helper)
    { }
};

typedef list<INSTANCE> DB_CMD;

static
DB_CMD & get_db() {
    static DB_CMD db_cmd;
    return db_cmd;
}

static
bool is_cli_inited = false;

static F_OUTPUT f_output = 0;
static F_CMD_LINK f_link = 0;
void UTILITY::cmd_set_op(F_CMD_LINK _f_link, F_OUTPUT _f_output)
{
    ASSERT(_f_link);
    f_link = _f_link;

    f_output = _f_output;

    is_cli_inited = true;

    DB_CMD::iterator it;
    DB_CMD &db = get_db();

    for(it=db.begin(); it!=db.end(); it++) {
        f_link(it->key, it->handler, it->helper);
    }

    db.clear();
}


CMD::AUTO_LINK::AUTO_LINK(const string &key, const HANDLER &handler, const HELPER &helper)
{
    if(is_cli_inited) {
        f_link(key, handler, helper);
    } else {
        get_db().push_back(INSTANCE(key, handler, helper));
    }
}


#include <mutex_u.h>
struct CMD_PRINTF_T { };
#define AUTO_MUTEX_CMD_PRT AUTO_MUTEX<CMD_PRINTF_T> _auto_mutex;

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
void CMD::printf(const char *fmt, ...)
{
    static char buff[4096];

    AUTO_MUTEX_CMD_PRT;

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





