
#ifndef TIMER_TEST_MOCK_H_
#define TIMER_TEST_MOCK_H_

typedef void (*sal_dpc_fn_t)(void *owner, void *, void *, void *, void *);

int dpc_time(unsigned int usec,
             sal_dpc_fn_t fn,
             void *owner, void *p1, void *p2, void *p3, void *p4);

void timer_op(void *ptr);

void pre_op();
void post_op();

#endif
