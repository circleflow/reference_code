
#ifndef TYPE_EXT_H
#define TYPE_EXT_H

#include "type.h"
using namespace CIRCLE_FLOW;

#include "utility/export/base_type.h"
using namespace UTILITY;

using std::vector;
#include <utility>
using std::make_pair;

typedef vector<bool> BITS;

struct PORT_ID{
    UINT8 unit;
    UINT8 index;

    PORT_ID(UINT8 _unit = 0, UINT8 _index = 0) : unit(_unit),index(_index) {}
    bool operator == (const PORT_ID & ref) const
        {return unit==ref.unit && index==ref.index;}
    bool operator != (const PORT_ID & ref) const
        { return !(operator == (ref)); }
    bool operator < (const PORT_ID &ref) const
        {return make_pair(unit,index) < make_pair(ref.unit, ref.index);}
};



#endif /* TYPE_EXT_H */
