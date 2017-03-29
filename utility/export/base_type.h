
#ifndef UTILITY_BASE_TYPE_H_
#define UTILITY_BASE_TYPE_H_

#ifdef ENV_C11
#include <cstdint>
#endif

namespace UTILITY {

#ifdef ENV_C11


    typedef uint8_t  UINT8;
    typedef uint16_t UINT16;
    typedef uint32_t UINT32;
    typedef uint64_t UINT64;

#else

    typedef unsigned char  UINT8;
    typedef unsigned short UINT16;
    typedef unsigned int   UINT32;
    typedef unsigned long long UINT64;

#endif

}


#endif /* UTILITY_BASE_TYPE_H_ */
