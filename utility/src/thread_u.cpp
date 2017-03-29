#include "thread_u.h"
#include "_dependency.h"
#include "error.h"
using namespace UTILITY;


//default implementation of dependency interface for thread, based on C++11
#ifdef ENV_C11
#include <thread>
using std::thread;

static
void * _create_thread(const string &, UINT8, UINT32)
{
    return new thread();
}

static
void _delete_thread(void * t)
{
    delete reinterpret_cast<thread*>(t);
}

static
void _run_thread(void *_t, const THREAD::RUN_OP &op)
{
    thread t(op);
    reinterpret_cast<thread*>(_t)->swap(t);
}

static F_THREAD_CREATE create_thread = _create_thread;
static F_THREAD_DELETE delete_thread = _delete_thread;
static F_THREAD_RUN    run_thread    = _run_thread;
static bool is_set = true;

#else

static F_THREAD_CREATE create_thread = 0;
static F_THREAD_DELETE delete_thread = 0;
static F_THREAD_RUN    run_thread    = 0;
static bool is_set = false;

#endif

#define ASSERT_READY ENSURE(true == is_set);

void UTILITY::thread_set_op(F_THREAD_CREATE _create,
                            F_THREAD_DELETE _delete,
                            F_THREAD_RUN    _run)
{
    ASSERT(_create && _delete && _run);

    create_thread = _create;
    delete_thread = _delete;
    run_thread    = _run;

    is_set = true;
}

struct THREAD::IMPL {

    string name;
    UINT8  pri;
    UINT32 stack;

    void *p_thread;

    IMPL(const string &_name,
         UINT8 _pri,
         UINT32 _stack)
    :name(_name),
     pri(_pri),
     stack(_stack),
     p_thread(0)
    { }

    ~IMPL()
    {
        ASSERT_READY;

        if(p_thread) {
            delete_thread(p_thread);
        }
    }

    void run(const RUN_OP &op)
    {
        ASSERT_READY;

        ENSURE(0==p_thread);

        ENSURE(p_thread = create_thread(name, pri, stack));
        run_thread(p_thread, op);
    }

};

THREAD::THREAD(const string &name,
               UINT8 pri,
               UINT32 stack)
{
    pimpl = new IMPL(name, pri, stack);
}

THREAD::~THREAD()
{
    delete pimpl;
}

void THREAD::run(const RUN_OP &op)
{
    pimpl->run(op);
}


