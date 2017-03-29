
#include "timer_mock.h"


TIMER::TIMER()
{
    timer_construct(this);
}

TIMER::~TIMER()
{
    timer_destruct(this);
}

void TIMER::set_op(const CLIENT_OP &op)
{
    timer_set_op(op);
}

void TIMER::start(UINT32 us, bool is_loop)
{
    timer_start(us, is_loop);
}

void TIMER::stop(void)
{
    timer_stop();
}

TIMER::TIMER(GUARD_OP pre_guard, GUARD_OP post_guard)
{
    timer_construct(this);
}
