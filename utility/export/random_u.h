
#ifndef UTILITY_RANDOM_H_
#define UTILITY_RANDOM_H_

#include "base_type.h"

namespace UTILITY {

    //range optimized random generator
    class RAND {
    public:
        operator UINT8  (void);
        operator UINT16 (void);
        operator UINT32 (void);
    };
}


#endif /* UTILITY_RANDOM_H_ */
