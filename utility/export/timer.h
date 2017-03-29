
#ifndef EXPORT_TIMER_NEW_H_
#define EXPORT_TIMER_NEW_H_

#include "base_type.h"
#include "function_bind.h"

namespace UTILITY {

    /*note: the callback would happens in another thread, keep in mind of data race safe
     *any exception thrown by the callback, would be implicitly caught*/

    class TIMER {
    public:
        typedef function< void (void) > CLIENT_OP;

        TIMER();
        ~TIMER();

        void set_op(const CLIENT_OP &op);

        void start(UINT32 after_ms, bool is_periodical = false);
        void stop(void);

        class IMPL;
        typedef void (*GUARD_OP) () ;

    protected:
        TIMER(GUARD_OP pre_guard, GUARD_OP post_guard);

        IMPL *pimpl;
    };


    /* considering data race safe,
     * the timer instance maybe changed(or destroyed) in parallel with TIMER callback thread
     * in this case, a pair of pre_op/post_op (typically mutex lock/unlock)
     * were introduced and placed around instance handling in callback thread.
     * pls note, client code also need using the same lock/unlock around its instance handling. */

    template<TIMER::GUARD_OP pre_guard, TIMER::GUARD_OP post_guard>
    class TIMER_GUARD : public TIMER
    {
    public:
        TIMER_GUARD()
        : TIMER(pre_guard, post_guard)
        { }
    };
}


#endif /* EXPORT_TIMER_NEW_H_ */
