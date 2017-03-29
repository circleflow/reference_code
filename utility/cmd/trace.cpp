
#include "trace.h"
#include "error.h"
#include "cmd.h"
using namespace UTILITY;

/* cmd to control trace output*/

static
void helper( void )
{
    static const char *help  =
           "\r\n trace enable|disable module_name --- enable or disable specific module"
           "\r\n       show [module_name]         --- display specific modules or all enabled modules"
           "\r\n";

    CMD::printf(help);
}

static
void dump(const vector<string> &_list)
{
    for(vector<string>::const_iterator it=_list.begin(); it!=_list.end(); it++) {
        CMD::printf(" %s ", it->c_str());
    }
}

static
void handler(const CMD::INPUTS &inputs )
{
    EXP_FREE_START;

    size_t size=inputs.size(),index=0;

    ENSURE(size>index);

    if( "show" == inputs[index] ) {

        if( size>(++index)) {
            CMD::printf("\r\n %s %s",
                    inputs[index].c_str(),
                    TRACE_FILTER::is_enabled(inputs[index].c_str()) ? "enabled" : "disabled");
        } else {
            CMD::printf("\r\n below modules enabled:");
            dump(TRACE_FILTER::get_enabled());

            CMD::printf("\r\n below modules disabled:");
            dump(TRACE_FILTER::get_disabled());
        }

    } else if("enable" == inputs[index]) {

        ENSURE(size>(++index));
        TRACE_FILTER::enable(inputs[index].c_str());
        CMD::printf("\r\n %s enabled", inputs[index].c_str());

    }else if("disable" == inputs[index]) {

        ENSURE(size>(++index));
        TRACE_FILTER::disable(inputs[index].c_str());
        CMD::printf("\r\n %s disabled", inputs[index].c_str());

    }

    EXP_FREE_END_NR;
}

static CMD::AUTO_LINK __cmd("trace", handler, helper);

