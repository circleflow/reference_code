
#include "time_u.h"
#include "wait.h"
using namespace UTILITY;


WAIT::WAIT(UINT32 ms)
{
    TIME::sleep_ms(ms);
}

#define CHECK_INTERVAL 10 //ms

WAIT::WAIT(UINT32 ms, const EXIT_CONDITION &exit_cond)
{
    if(ms<CHECK_INTERVAL) {
        TIME::sleep_ms(ms);
        return;
    }

    int i = (ms+CHECK_INTERVAL-1)/CHECK_INTERVAL;

    while(i>0) {

        TIME::sleep_ms(CHECK_INTERVAL);

        if(exit_cond()) {
            return;
        }

        i--;
    }
}

