#include "mutex_u.h"
using namespace UTILITY;


MUTEX::MUTEX()
{ }

MUTEX::~MUTEX()
{ }

void MUTEX::lock()
{ }

bool MUTEX::try_lock(UINT32)
{
    return true;
}

void MUTEX::unlock()
{ }

