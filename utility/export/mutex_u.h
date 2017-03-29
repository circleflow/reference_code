

#ifndef UTILITY_AUTO_MUTEX_H_
#define UTILITY_AUTO_MUTEX_H_

#include <string>
using std::string;
#include "base_type.h"

namespace UTILITY {

    class MUTEX {
    public:
        MUTEX();
        ~MUTEX();

        void lock();    //wait for ever
        bool try_lock(UINT32 expired_ms);   //return true: locked; false: timeout;
        void unlock();
    };

    struct LOCK_GUARD {
        LOCK_GUARD(MUTEX & m)
        :_m(m)
        {
            _m.lock();
        }

        ~LOCK_GUARD()
        {
            _m.unlock();
        }

    private:
        MUTEX & _m;
    };

    // type based mutex, with static member function lock/unlock
    template <typename T>
    class AUTO_MUTEX
    {
    public:

        AUTO_MUTEX ()
        { m.lock(); }

        ~AUTO_MUTEX()
        { m.unlock(); }

        static void lock()
        { m.lock(); }

        static void unlock()
        { m.unlock(); }

    private:
        static MUTEX m;
    };

    template <typename T>
    MUTEX AUTO_MUTEX<T>::m;
}



#endif /* UTILITY_AUTO_MUTEX_H_ */
