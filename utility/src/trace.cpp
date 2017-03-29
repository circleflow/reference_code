#include "_dependency.h"
#include "trace.h"
using namespace UTILITY;

#include "mutex_u.h"
struct TRC_MUTEX { };
#define AUTO_MUTEX_TRC AUTO_MUTEX<TRC_MUTEX> _auto_mutex

#include "error.h"

#include <vector>
using std::vector;
#include <map>
using std::map;
#include <exception>

#include <iostream>
#include <stdio.h>
#include <stdarg.h>

static F_OUTPUT f_output=0;

void UTILITY::trace_set_output(F_OUTPUT f)
{
    f_output = f;
}

void TRC::printf(const char *fmt, ...)
{
    static char buff[4096];

    AUTO_MUTEX_TRC;

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


struct TRACE_BASE::INFO{
    bool is_dumped;
    vector<TRACE_BASE *> objs;

    INFO():is_dumped(false) { }
};


#ifndef ENV_C11
extern "C" {
#include "sal/core/thread.h"
}
#endif

static
TRACE_BASE::INFO & get_info()
{
#ifdef ENV_C11
    thread_local  TRACE_BASE::INFO db;
    return db;
#else
    /* here it depends on BCM SDK specifically, instead of depending on common interface, like
     *  THREAD::get_id_of_context() or something.
     * there's some difficulty to encapsulate the type of thread_id, because its type varies among OS.
     * we could define interface for allow inject implementation, but not easy for type to be injected.
     * in simple, we leave the dependency on BCM SDK here, which have encapsulate the OS difference.
     * */
    static map<sal_thread_t, TRACE_BASE::INFO> db_map;
    return db_map[sal_thread_self()];
#endif
}

void TRACE_BASE::dump_all()
{
    dump_all(get_info());
}

void TRACE_BASE::dump_all(INFO &info)
{
    TRC::printf("\r\n === start of trace stack === \r\n");
    for(int i=info.objs.size()-1; i>=0; i--) {
        info.objs[i]->dump();
    }
    TRC::printf("\r\n === end of trace stack === \r\n");
}

TRACE_BASE::TRACE_BASE()
:info(get_info())
{
    if(info.is_dumped && (! std::uncaught_exception())) {
        info.is_dumped = false;
    }

    info.objs.push_back(this);
}

void TRACE_BASE::on_destroy()
{
    if(std::uncaught_exception() && (! info.is_dumped)){
        info.is_dumped = true;
        dump_all(info);
    }
}

TRACE_BASE::~TRACE_BASE()
{
    info.objs.pop_back();
}


typedef map<string, TRACE_FILTER *> FILTER_DB;

static
FILTER_DB & get_filter_db()
{
    static FILTER_DB db;
    return db;
}

static
TRACE_FILTER * get_filter(const char *name)
{
    FILTER_DB::iterator it = get_filter_db().find(name);
    if(it != get_filter_db().end()) {
        return it->second;
    } else {
        return 0;
    }
}


TRACE_FILTER::TRACE_FILTER(const char *_name)
: name(_name),
  m_is_enabled(false)
{
    get_filter_db()[string(name)] = this;
}

TRACE_FILTER::~TRACE_FILTER()
{
    get_filter_db().erase(name);
}

void TRACE_FILTER::enable()
{
    m_is_enabled = true;
}

void TRACE_FILTER::disable()
{
    m_is_enabled = false;
}

void TRACE_FILTER::enable(const char * name)
{
    TRACE_FILTER *filter = get_filter(name);
    if(filter) {
        filter->enable();
    }
}

void TRACE_FILTER::disable(const char * name)
{
    TRACE_FILTER *filter=get_filter(name);
    if(filter) {
        filter->disable();
    }
}

bool TRACE_FILTER::is_enabled() const
{
    return m_is_enabled;
}

bool TRACE_FILTER::is_enabled(const char *name)
{
    TRACE_FILTER *filter = get_filter(name);
    ENSURE(filter);

    return filter->is_enabled();
}

vector<string> TRACE_FILTER::get_enabled()
{
    vector<string> result;

    FILTER_DB &db = get_filter_db();
    for(FILTER_DB::iterator it=db.begin(); it!=db.end(); it++) {
        if(it->second->is_enabled()) {
            result.push_back(it->second->name);
        }
    }

    return result;
}

vector<string> TRACE_FILTER::get_disabled()
{
    vector<string> result;

    FILTER_DB &db = get_filter_db();
    for(FILTER_DB::iterator it=db.begin(); it!=db.end(); it++) {
        if(!(it->second->is_enabled())) {
            result.push_back(it->second->name);
        }
    }

    return result;
}

DURATION_TRACE::DURATION_TRACE(const TRACE_FILTER &_filter, const char *_str)
:filter(_filter),
 str(_str)
{
    if(filter.is_enabled()) {
        dump();
    }
}

void DURATION_TRACE::dump(void)
{
    TRC::printf("\r\n %s started", str);
}

DURATION_TRACE::~DURATION_TRACE()
{
    on_destroy();

    if(filter.is_enabled()) {
        TRC::printf("\r\n %s ended", str);
    }
}

TRACE_P0
UTILITY::make_trace(const TRACE_FILTER &filter, const char *fmt)
{
    TRACE_P0 trc(fmt);
    if(filter.is_enabled()) trc.dump();
    return trc;
}
