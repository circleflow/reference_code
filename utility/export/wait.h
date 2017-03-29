
#ifndef UTILITY_WAIT_H_
#define UTILITY_WAIT_H_

#include "base_type.h"
#include "function_bind.h"

namespace UTILITY {

    //block current thread by a specified period of time,
    //or either exit under certain condition
    class WAIT {
    public:
        WAIT(UINT32 ms);

        typedef function< bool (void) > EXIT_CONDITION;

        WAIT(UINT32 ms, const EXIT_CONDITION &);
    };

}

#endif
