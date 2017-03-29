
#include "random_u.h"
using namespace UTILITY;

#include <cstdlib>

RAND::operator UINT8(void)
{
    return (double(rand())/RAND_MAX)*0xff;
}

RAND::operator UINT16(void)
{
    return (this->operator UINT8() << 8)
               | (this->operator UINT8() << 0);
}

RAND::operator UINT32(void)
{
    return (this->operator UINT16() << 16)
               | (this->operator UINT16() << 0);
}

