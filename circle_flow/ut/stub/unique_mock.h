
#ifndef UNIQUE_MOCK_H
#define UNIQUE_MOCK_H

#include "utility/export/unique.h"
using namespace UTILITY;

void unique_construct(const string &pool);
void unique_destruct();
UINT16 unique_get_id(void);


#endif
