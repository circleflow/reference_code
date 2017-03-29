
#ifndef UDF_H_
#define UDF_H_

#include "type_ext.h"
#include "qual.h"

class UDF {
public:
    UDF (UINT8 unit, UINT8 offset, UINT8 size_of_byte);
    ~UDF();

    void set_match(const QUAL::MATCH &match);
    QUAL::MATCH get_match() const;

private:

    UDF ();
    UDF (const UDF &ref);
    UDF & operator = (const UDF &ref);

    struct IMPL;
    IMPL *pimpl;
};


#endif /* UDF_H_ */
