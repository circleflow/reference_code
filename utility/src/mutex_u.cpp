
#include "mutex_u.h"
#include "error.h"
#include "_dependency.h"
using namespace UTILITY;

#include <map>
using std::map;

//default implementation of dependency interface for mutex, based on C++11
#ifdef ENV_C11

#include <mutex>
#include <chrono>

#define MUTEX_IMPL std::timed_mutex

static
void * _mutex_create()
{
    return new MUTEX_IMPL();
}

static
void _mutex_delete(void * m)
{
    delete reinterpret_cast<MUTEX_IMPL *> (m);
}

static
void _mutex_lock(void * m)
{
    reinterpret_cast<MUTEX_IMPL *>(m)->lock();
}

static
bool _mutex_try_lock(void *m, unsigned int expired_ms)
{
    return reinterpret_cast<MUTEX_IMPL *>(m)->try_lock_for(std::chrono::milliseconds(expired_ms));
}

static
void _mutex_unlock(void * m)
{
    reinterpret_cast<MUTEX_IMPL *>(m)->unlock();
}

#endif


#define MUTEX_HANDLER void *
#define WAIT_FOR_EVER 0


#ifdef ENV_C11

static F_MUTEX_CREATE   mutex_create   = _mutex_create;
static F_MUTEX_DELETE   mutex_delete   = _mutex_delete;
static F_MUTEX_LOCK     mutex_lock     = _mutex_lock;
static F_MUTEX_TRY_LOCK mutex_try_lock = _mutex_try_lock;
static F_MUTEX_UNLOCK   mutex_unlock   = _mutex_unlock;

static MUTEX_HANDLER m_db = _mutex_create();
static bool is_set = true;

#else
static F_MUTEX_CREATE   mutex_create   = 0;
static F_MUTEX_DELETE   mutex_delete   = 0;
static F_MUTEX_LOCK     mutex_lock     = 0;
static F_MUTEX_TRY_LOCK mutex_try_lock = 0;
static F_MUTEX_UNLOCK   mutex_unlock   = 0;

static MUTEX_HANDLER m_db;  //for db access
static bool is_set = false;

#endif

void UTILITY::mutex_set_op(F_MUTEX_CREATE    _create,
                           F_MUTEX_DELETE    _delete,
                           F_MUTEX_LOCK      _lock,
                           F_MUTEX_TRY_LOCK  _try_lock,
                           F_MUTEX_UNLOCK    _unlock)
{
    ASSERT(_create);
    ASSERT(_delete);
    ASSERT(_lock);
    ASSERT(_try_lock);
    ASSERT(_unlock);

    mutex_create   = _create;
    mutex_delete   = _delete;
    mutex_lock     = _lock;
    mutex_try_lock = _try_lock;
    mutex_unlock   = _unlock;

    ASSERT(m_db=mutex_create());

    is_set = true;
}

#define ASSERT_READY ASSERT(true == is_set);

typedef map<MUTEX *, MUTEX_HANDLER> DB_MUTEX;

static
DB_MUTEX & get_db()
{
    /* defined as pointer instead of an object
       because as an object, it will be destructed before other global/static MUTEX instance,
       which lead to error when program exit*/
    static DB_MUTEX *db=new DB_MUTEX();
    return *db;
}

static
MUTEX_HANDLER get_handler(MUTEX *p)
{
    //handler creation protect
    struct _AUTO_MUTEX {
        _AUTO_MUTEX()  { mutex_lock(m_db);   }
        ~_AUTO_MUTEX() { mutex_unlock(m_db); }
    } _auto_mutex;

    DB_MUTEX & db = get_db();
    DB_MUTEX::iterator it = db.find(p);

    //create if not exist
    if(it == db.end()) {
        MUTEX_HANDLER m=mutex_create();
        ENSURE(m>0);

        db[p]=m;
        return m;
    }else{
        return it->second;
    }
}

void MUTEX::lock ()
{
    ASSERT_READY;

    MUTEX_HANDLER m=get_handler(this);

     mutex_lock(m);
}

bool MUTEX::try_lock(UINT32 expired_ms)
{
    ASSERT_READY;

    MUTEX_HANDLER m=get_handler(this);

    return mutex_try_lock(m, expired_ms);
}

void MUTEX::unlock()
{
    ASSERT_READY;

    MUTEX_HANDLER m=get_handler(this);

    mutex_unlock(m);
}

MUTEX::MUTEX()
{ /*nothing*/ }

MUTEX::~MUTEX()
{
    ASSERT_READY;

    DB_MUTEX &db = get_db();
    DB_MUTEX::iterator it = db.find(this);

    if(it != db.end()) {
        mutex_delete(it->second);
        db.erase(it);
    }
}
