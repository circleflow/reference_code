
#include "unique_mock.h"


UNIQUE::UNIQUE(const string &pool)
{
    unique_construct(pool);
}

UNIQUE::~UNIQUE()
{
    unique_destruct();
}

UINT16 UNIQUE::get_id(void) const
{
    return unique_get_id();
}
