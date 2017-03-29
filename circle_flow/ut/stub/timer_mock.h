#ifndef TIMER_MOCK_H
#define TIMER_MOCK_H

#include "utility/export/base_type.h"
#include "utility/export/timer.h"
using namespace UTILITY;

void timer_construct(TIMER *timer);
void timer_destruct(TIMER *timer);
void timer_set_op(const TIMER::CLIENT_OP &op);
void timer_start(UINT32 us, bool is_loop);
void timer_stop();

#endif
