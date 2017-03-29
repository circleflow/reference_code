
#include "time_u.h"
#include "_dependency.h"
#include "error.h"
using namespace UTILITY;


//default implementation of dependency interface for time
#ifdef ENV_C11

#include <chrono>
using namespace std::chrono;

static
UINT32 impl_get_elapsed_ms()
{
    auto time_now = system_clock::now();
    auto ms = duration_cast<milliseconds>(time_now.time_since_epoch());

    return static_cast<UINT32>(ms.count());
}


static
UINT32 impl_max_elapsed_ms()
{
    auto ms_max = milliseconds::max().count();

    if(ms_max > UINT32_MAX) {
        return UINT32_MAX;
    } else {
        return ms_max;
    }
}


#include <thread>
static
void impl_sleep_ms(UINT32 ms)
{
    std::this_thread::sleep_for (milliseconds(ms));
}

static F_TIME_GET_ELAPSED _get_elapsed_ms = impl_get_elapsed_ms;
static F_TIME_MAX_ELAPSED _max_elapsed_ms = impl_max_elapsed_ms;
static F_TIME_SLEEP _sleep_ms = impl_sleep_ms;

static bool is_set = true;

#else

static F_TIME_GET_ELAPSED _get_elapsed_ms = 0;
static F_TIME_MAX_ELAPSED _max_elapsed_ms = 0;
static F_TIME_SLEEP _sleep_ms = 0;

static bool is_set = false;

#endif

#define ASSERT_READY ENSURE(true == is_set);

UINT32 UTILITY::TIME::get_elapsed_ms()
{
    ASSERT_READY;

    return _get_elapsed_ms();
}

UINT32 UTILITY::TIME::max_elapsed_ms()
{
    ASSERT_READY;

    return _max_elapsed_ms();
}

void UTILITY::TIME::sleep_ms(UINT32 ms)
{
    ASSERT_READY;

    _sleep_ms(ms);
}

void UTILITY::time_set_op(F_TIME_GET_ELAPSED _get_elapsed,
                          F_TIME_MAX_ELAPSED _max_elapsed,
                          F_TIME_SLEEP _sleep)
{
    ENSURE(_get_elapsed && _max_elapsed && _sleep);

    _get_elapsed_ms = _get_elapsed;
    _max_elapsed_ms = _max_elapsed;
    _sleep_ms = _sleep;

    is_set = true;
}
