#include "utility/export/error.h"
#include "utility/export/_dependency.h"
using namespace UTILITY;


extern "C" {
#include "sal/core/sync.h"
}

#define MUTEX_HANDLER sal_sem_t

static
void * mutex_create()
{
    char name[] = "usem";
    MUTEX_HANDLER m=sal_sem_create(name, sal_sem_BINARY, 1);
    ENSURE(m>0);

    return m;
}

static
void mutex_delete(void * m)
{
    sal_sem_destroy((MUTEX_HANDLER)m);
}

static
void mutex_lock(void * m)
{
    ENSURE_OK(sal_sem_take((MUTEX_HANDLER)m, sal_sem_FOREVER));
}

static
bool mutex_try_lock(void * m, UINT32 expired_ms)
{
    int r = sal_sem_take((MUTEX_HANDLER)m, expired_ms*1000);
    ENSURE(0==r || -1==r, " r=%d ", r);

    return 0==r ? true : false;
}

static
void mutex_unlock(void * m)
{
    ENSURE_OK(sal_sem_give((MUTEX_HANDLER)m));
}


namespace SYS_ADAPT {
    void mutex_init(void)
    {
        mutex_set_op(mutex_create,
                     mutex_delete,
                     mutex_lock,
                     mutex_try_lock,
                     mutex_unlock);
    }
}
