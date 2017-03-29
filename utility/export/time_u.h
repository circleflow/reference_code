
#ifndef UTILITY_TIME_H_
#define UTILITY_TIME_H_

#include "base_type.h"

namespace UTILITY {

    namespace TIME {

        UINT32 get_elapsed_ms();   //in millisecond
        UINT32 max_elapsed_ms();   //upper boundary of elapsed milliseconds, may device depends
        void sleep_ms(UINT32 ms);  //in millisecond
    }
}

#endif /* UTILITY_TIME_H_ */
