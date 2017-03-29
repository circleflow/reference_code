
#ifndef FP_ENTRY_H_
#define FP_ENTRY_H_

extern "C" {
#include "bcm/field.h"
}

#include "utility/export/error.h"

class FP_ENTRY {
public:
    FP_ENTRY(int _unit, int gid)
    : unit(_unit), eid(-1)
    {
        ENSURE_OK(
            bcm_field_entry_create(unit, gid, &eid));
    }

    FP_ENTRY(const FP_ENTRY &entry)
    : unit(entry.unit)
    {
        ENSURE_OK(
            bcm_field_entry_copy(unit, entry.eid, &eid));
    }

    ~FP_ENTRY()
    {
        ENSURE_OK(
            bcm_field_entry_destroy(unit,eid));
    }

    int get_eid()
    {
        return eid;
    }


private:
    int unit, eid;
};


#endif /* FP_ENTRY_H_ */
