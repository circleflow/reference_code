
#ifndef EXPORT__DEPENDENCY_H_
#define EXPORT__DEPENDENCY_H_

#include "thread_u.h"
#include "cmd.h"
#include "base_type.h"

namespace UTILITY {

    /* all system dependency are defined below  */

    typedef void (*F_OUTPUT) (const char *str);

    /* === mandatory dependency interface ===
     * must be implemented accordingly upon target system,
     * and registered at beginning of initialization */

    //cmd
    typedef void (*F_CMD_LINK) (const string &key, const CMD::HANDLER &, const CMD::HELPER &);
    void cmd_set_op(F_CMD_LINK, F_OUTPUT f=0);    //output use printf by default

    /* === optional dependency interface ===
      * there's a default implementation based on c++11 stdlib
      * to enable the default implementation, simply define compile flag: ENV_C11
      * it also could be override by customized implementation at runtime  */

    //error
    void error_set_output(F_OUTPUT);    //redirect the error info output, using printf by default

    typedef void (*F_TERMINATE) (const char *file, int line, const char *str);
    void error_set_terminate(F_TERMINATE);  //std::terminate by default

    //trace
    void trace_set_output(F_OUTPUT);  //redirect the trace info output, using printf by default

    //mutex
    typedef void * (*F_MUTEX_CREATE)();
    typedef void   (*F_MUTEX_DELETE)(void *m);
    typedef void   (*F_MUTEX_LOCK)  (void *m);
    typedef bool   (*F_MUTEX_TRY_LOCK)(void *m, UINT32 expired_ms);//true: lock ok; false: timeout
    typedef void   (*F_MUTEX_UNLOCK)(void *m);

    void mutex_set_op(F_MUTEX_CREATE,
                      F_MUTEX_DELETE,
                      F_MUTEX_LOCK,
                      F_MUTEX_TRY_LOCK,
                      F_MUTEX_UNLOCK);

    //thread
    typedef void * (*F_THREAD_CREATE)(const string &name, UINT8 priority, UINT32 stack_size /*kbyte*/);
    typedef void   (*F_THREAD_DELETE)(void *thread);
    typedef void   (*F_THREAD_RUN)(void *thread, const THREAD::RUN_OP &op);

    void thread_set_op(F_THREAD_CREATE,
                       F_THREAD_DELETE,
                       F_THREAD_RUN);

    //time
    typedef UINT32 (*F_TIME_GET_ELAPSED)();
    typedef UINT32 (*F_TIME_MAX_ELAPSED)();
    typedef void   (*F_TIME_SLEEP)(UINT32 ms);

    void time_set_op(F_TIME_GET_ELAPSED,
                     F_TIME_MAX_ELAPSED,
                     F_TIME_SLEEP);

}

#endif /* EXPORT__DEPENDENCY_H_ */
