
#ifndef SRC_HELPER_H_
#define SRC_HELPER_H_


#define EXP_FREE_START_LUA_C \
    try {

#define EXP_FREE_END_LUA_C \
    }\
    catch (exception &e) {\
        lua_pushnumber(L, 1);\
        lua_pushstring(L, e.what());\
    }\
    catch (...) {\
        lua_pushnumber(L, 1);\
        lua_pushstring(L, "unknown error");\
    }

#include "utility/export/trace.h"
namespace CIRCLE_FLOW_LUA_C {
    extern UTILITY::TRACE_FILTER _trc_filter;
}

#ifndef ENV_UT
#define TRACE(args...) _TRACE(CIRCLE_FLOW_LUA_C::_trc_filter, args)
#else
#include <stdio.h>
#define TRACE(args...) printf(args)
#endif

#include <string.h>
#include <string>
using std::string;
#include <sstream>
using std::ostream;
using std::ostringstream;
using std::endl;

template<class T> string string_any(T t)
{
    ostringstream oss;
    oss<<t;
    return oss.str();
}

template<class T> string left_padding(T t, unsigned int size)
{
    string str(string_any(t));

    const int max_padding=20;
    static const char padding [max_padding+1] = "                    ";
    const char * ptr_end = padding + max_padding;

    int padding_size = (int)size - (int)strlen(str.c_str());

    if(padding_size <= 0) {
        return str;
    } else if(padding_size >= max_padding) {
        return string(padding) + str;
    } else {
      return string(ptr_end-padding_size) + str;
    }
}

//#define ENV_UT 1

#define ARRAY_NUM(array) (sizeof(array)/sizeof(array[0]))

#endif
