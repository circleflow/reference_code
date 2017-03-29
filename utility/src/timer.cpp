#include "thread_u.h"
#include "time_u.h"
#include "timer.h"
#include "mutex_u.h"
#include "error.h"
using namespace UTILITY;

#include <map>
using std::map;
using std::pair;
using std::make_pair;
#include <list>
using std::list;
#include <algorithm>
using std::find_if;

#include "trace.h"
static
TRACE_FILTER trc_filter_verbose("timer");

#define TRACE(args...) _TRACE(trc_filter_verbose, args)


/* to have the GUARD_OP protection before fetching the timer instance from list,
 * a dedicated callback thread is created for each dedicated type of GUARD_OP. */

/* we presume below 2 scenarios while considering data race safe
 * 1: with GUARD_OP, timer instance would be changed within or in parallel with callback thread
 * 2: without GUARD_OP, timer instance would only be changed within callback thread */

struct TIMER_THREAD {

    static TIMER_THREAD * get_instance(TIMER::GUARD_OP pre_op, TIMER::GUARD_OP post_op);

    TIMER_THREAD(TIMER::GUARD_OP pre_op, TIMER::GUARD_OP post_op);

    void insert(UINT32 after_ms, TIMER::IMPL *);
    bool remove(TIMER::IMPL *); //true: removed; false: not find

    //structure of timer db element
    struct ELEMENT {
        /* choosing the relative timestamp instead of absolute timestamp
         * it will benefit to
         * > the range of timer could up to 2^32 ms, more than 1 month, meet the need of most
         * > simplify the process when timestamp reverse */
        UINT32 delta_ms;

        TIMER::IMPL * timer;

        ELEMENT(UINT32 _delta_ms=0, TIMER::IMPL * _timer=0)
        :delta_ms(_delta_ms),
         timer(_timer)
        { }
    };

private:
    TIMER::GUARD_OP pre_op, post_op;

    typedef list< ELEMENT > DB_TIMER;
    DB_TIMER db;

    UINT32 time_last;

    MUTEX m_db;

    THREAD cb_thread;
    MUTEX m_sync;   //ensure destruction after thread, because thread is running on it

    void thread_run();
};

struct TIMER::IMPL {
    CLIENT_OP op;

    UINT32 after_ms;

    bool is_periodical;
    bool is_started;

    TIMER_THREAD *cb_thread;

    IMPL(GUARD_OP pre_op, GUARD_OP post_op)
    : after_ms(0),
      is_periodical(false),
      is_started(false),
      cb_thread(TIMER_THREAD::get_instance(pre_op, post_op))
    {  }

    void set_op(const CLIENT_OP &_op)
    {
        ENSURE(false == is_started);
        op = _op;
    }

    void start(UINT32 _after_ms, bool _is_periodical)
    {
        ENSURE(op); //make sure op has been set up
        ENSURE(false == is_started);

        after_ms = _after_ms;
        is_periodical = _is_periodical;

        cb_thread->insert(after_ms, this);

        is_started = true;
    }

    void stop(void)
    {
        if(is_started) {
            is_started = false;
            cb_thread->remove(this);
        }
    }

    ~IMPL()
    {
        stop();
    }

};

TIMER::TIMER()
{
    pimpl = new IMPL(0, 0);
}

TIMER::TIMER(GUARD_OP pre_guard, GUARD_OP post_guard)
{
    pimpl = new IMPL(pre_guard, post_guard);
}

TIMER::~TIMER()
{
    delete pimpl;
}

void TIMER::set_op(const CLIENT_OP &op)
{
    pimpl->set_op(op);
}

void TIMER::start(UINT32 after_ms, bool is_periodical)
{
    pimpl->start(after_ms, is_periodical);
}

void TIMER::stop(void)
{
    pimpl->stop();
}

typedef map< pair<TIMER::GUARD_OP,TIMER::GUARD_OP>, TIMER_THREAD* > DB_THREAD;

TIMER_THREAD * TIMER_THREAD::get_instance(TIMER::GUARD_OP pre_op, TIMER::GUARD_OP post_op)
{
    static  DB_THREAD db;

    AUTO_MUTEX<TIMER_THREAD> auto_lock;

    TIMER_THREAD *instance;
    DB_THREAD::iterator it = db.find(make_pair(pre_op, post_op));
    if(db.end() == it) {
        instance = new TIMER_THREAD (pre_op, post_op);
        db.insert(make_pair(make_pair(pre_op,post_op), instance));
    } else {
        instance = it->second;
    }

    return instance;
}

TIMER_THREAD::TIMER_THREAD(TIMER::GUARD_OP _pre_op, TIMER::GUARD_OP _post_op)
:pre_op(_pre_op),
 post_op(_post_op),
 cb_thread("timer", 50, 64) //priority 50, stack size 64kb
{
    m_sync.lock();

    cb_thread.run(bind(&TIMER_THREAD::thread_run, this));
}

/* the timer instance list (TIMER_THREAD::db) is arranged in ascend order of delta_ms
 * the meaning of delta_ms is incremental and relative, for example of a list (from begin to end):
 * instance A, delta_ms == 1 ms  --> after_ms is 1ms
 * instance B, delta_ms == 2 ms  --> after_ms is 1+2=3ms
 * instance C, delta_ms == 0 ms  --> after_ms is 1+2+0=3ms
 * instance D, delta_ms == 3 ms  --> after_ms is 1+2+0+3=6ms
 *
 * this organization helps to simplify the update to time base (TIMER_THREAD::time_last)
 * which will be changed frequently in thread_run, without need to update all instance in list except first one*/

static
bool is_timer_exist(TIMER::IMPL *timer, const TIMER_THREAD::ELEMENT &element)
{
    return timer == element.timer;
}

struct IS_TIMER_BEFORE {
    UINT32 &delta_ms;

    IS_TIMER_BEFORE(UINT32 &_delta_ms)
    :delta_ms(_delta_ms)
    { }

    bool operator () (const TIMER_THREAD::ELEMENT &element)
    {
        if(delta_ms >= element.delta_ms) {
            delta_ms -= element.delta_ms;
            return false;
        } else {
            return true;
        }
    }
};

void TIMER_THREAD::insert(UINT32 after_ms, TIMER::IMPL *timer)
{
    UINT32 time_now = TIME::get_elapsed_ms();   //placed before lock of m_db
    UINT32 time_max = TIME::max_elapsed_ms();   //to minimum the time period of locking

    LOCK_GUARD lock(m_db);

    DB_TIMER::iterator it;

    /*check if inserted already,
     *it could be possible in case of timer enabled again during callback,
     *bother client code in callback, and thread_run () after callback op, will insert timer into list*/
    it = find_if(db.begin(), db.end(), bind(&is_timer_exist, timer, placeholders::_1));
    if(it != db.end()) {
        TRACE("timer instance already existed. \r\n");
        return;
    }

    //each timer placed in ascend order of delta_ms, in list
    UINT32 time_delta;
    if (time_now < time_last) {
        time_delta = time_max - time_last + time_now;
    }else{
        time_delta = time_now - time_last;
    }

    time_delta += after_ms;

    IS_TIMER_BEFORE is_before(time_delta);
    it = find_if(db.begin(), db.end(), is_before);

    //update delta of next timer instance after this
    if(it!=db.end()) {
        TRACE("placed before another instance, with original delta_ms %d, ", it->delta_ms);
        it->delta_ms -= time_delta;
        TRACE("%d afterward \r\n", it->delta_ms);
    }

    db.insert(it, ELEMENT(time_delta, timer));

    //active callback thread, to refresh the timeout value
    if(db.begin()==it || 1==db.size()) {
        TRACE("active thread \r\n");
        m_sync.unlock();
    }
}

bool TIMER_THREAD::remove(TIMER::IMPL *timer)
{
    LOCK_GUARD lock(m_db);

    DB_TIMER::iterator it;

    it = find_if(db.begin(), db.end(), bind(is_timer_exist, timer, placeholders::_1));
    if(it != db.end()) {

        //update delta of next timer instance after this
        DB_TIMER::iterator it_next(it);
        it_next++;
        if(it_next != db.end()) {
            it_next->delta_ms += it->delta_ms;
        }

        db.erase(it);
        return true;
    } else {
        return false;
    }
}

void TIMER_THREAD::thread_run()
{
#define WAIT_STAND_BY 1000

    UINT32 expired_ms = WAIT_STAND_BY;
    UINT32 time_max = TIME::max_elapsed_ms();

    while(1) {

        TRACE("lock for %d @ %d \r\n", expired_ms, TIME::get_elapsed_ms());

        m_sync.try_lock(expired_ms);

        TRACE("locked       %d \r\n", TIME::get_elapsed_ms());

        //timer instance handling loop
        while(1) {

            //ensure no exit in case exception throws in callback
            EXP_FREE_START;

            //pre_op & post_op, automatically applies
            struct AUTO_GUARD_OP {
                AUTO_GUARD_OP(TIMER::GUARD_OP pre_op, TIMER::GUARD_OP post_op)
                :_post_op(post_op)
                {  if(pre_op)  pre_op();  }

                ~AUTO_GUARD_OP()
                {  if(_post_op) _post_op(); }

                TIMER::GUARD_OP _post_op;
            }  _guard(pre_op, post_op);

            //get elapsed time since last loop
            UINT32 time_now = TIME::get_elapsed_ms();   //placed before lock of m_db, minimum the scope of locking
            UINT32 time_delta;

            if (time_now < time_last) {
                time_delta = time_max - time_last + time_now;
            }else{
                time_delta = time_now - time_last;
            }

            TIMER::IMPL *timer=0;

            {   //enter db lock scope
                LOCK_GUARD lock(m_db);

                DB_TIMER::iterator it=db.begin();
                if(it != db.end()) {

                    if(it->delta_ms <= time_delta) {

                        //before remove this, update delta of next timer instance
                        DB_TIMER::iterator it_next(it);
                        it_next ++;
                        if(it_next != db.end()) {
                            it_next->delta_ms += it->delta_ms;
                        }

                        //get the timer for handling and remove it from list
                        timer = it->timer;
                        db.erase(it);

                        //skip if invalid
                        if((0==timer) || (!timer->is_started) || (!timer->op)) {
                            continue;
                        }

                        //mark timer status in case of non-periodical
                        if(!timer->is_periodical) {
                            timer->is_started = false;
                        }

                    } else {
                        /* no timer need handling,
                         * update delta of next timer instance, and wait*/
                        time_last = time_now;
                        it->delta_ms -= time_delta;
                        expired_ms = it->delta_ms;
                        break;
                    }

                }else{  //no timer exist
                    expired_ms = WAIT_STAND_BY;
                    break;
                }

            }   //leave db lock scope

            timer->op();

            if( timer->is_started && timer->is_periodical) {
                /* it is possible that timer already inserted itself during callback
                 * so the insert() must ensure no repeated insertion*/
                insert(timer->after_ms, timer);
            }

            EXP_FREE_END_NR;
        }
    }

}
