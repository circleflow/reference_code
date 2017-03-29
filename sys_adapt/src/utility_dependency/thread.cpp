#include "utility/export/error.h"
#include "utility/export/_dependency.h"
using namespace UTILITY;

//implementation upon demo board

extern "C" {
    #include <sal/core/thread.h>
}

#include <string.h>

struct THREAD_INFO {
    char name[5];
    int priority;
    int stack_size;
    sal_thread_t tid;

    THREAD_INFO(const string &_name, UINT8 _priority, UINT32 _stack_size)
    {
        strncpy(name, _name.c_str(), 4);
        name[4] = 0;
        priority = _priority;
        stack_size = _stack_size;
        tid = 0;
    }
};

static
void * create_thread(const string &name, UINT8 priority, UINT32 stack_size /*kbyte*/)
{
    THREAD_INFO *info = new THREAD_INFO(name, priority, stack_size);

    return info;
}

static
void delete_thread(void *thread)
{
    THREAD_INFO * info = reinterpret_cast<THREAD_INFO *>(thread);
    sal_thread_destroy(info->tid);

    delete info;
}

static
void thread_func(THREAD::RUN_OP *p_op)
{
    EXP_FREE_START;

    (*p_op)();

    EXP_FREE_END_NR;

    delete p_op;
}

static
void run_thread(void *thread, const THREAD::RUN_OP &op)
{
    THREAD_INFO * info = reinterpret_cast<THREAD_INFO *>(thread);

    THREAD::RUN_OP *p_op = new THREAD::RUN_OP(op);

    info->tid = sal_thread_create(info->name,
                                  info->stack_size,
                                  info->priority,
                                  (void (*)(void*))thread_func,
                                  p_op);

    ENSURE(SAL_THREAD_ERROR != info->tid);
}


namespace SYS_ADAPT {
    void thread_init(void)
    {
        thread_set_op(create_thread,
                      delete_thread,
                      run_thread);
    }
}
