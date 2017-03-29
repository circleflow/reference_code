
#ifndef BYTES_BITS_H_
#define BYTES_BITS_H_

#include "type_ext.h"

class BYTES_CONVERT {
public:
    BYTES_CONVERT(const BYTES &);

    operator UINT8  (void) const;
    operator UINT16 (void) const;
    operator UINT32 (void) const;
    operator int    (void) const;
    operator UINT8* (void);

    ~BYTES_CONVERT();

private:
    BYTES_CONVERT(const BYTES_CONVERT &);
    BYTES_CONVERT & operator = (const BYTES_CONVERT &);

    const BYTES bytes;
    UINT8 *str;
};

namespace CIRCLE_FLOW {
    BYTES make_bytes(UINT8);
    BYTES make_bytes(UINT16);
    BYTES make_bytes(UINT32);

    BYTES operator &  (const BYTES &, const BYTES &);
    void  operator &= (BYTES &, const BYTES &);
    BYTES operator |  (const BYTES &, const BYTES &);
    void  operator |= (BYTES &, const BYTES &);

    BITS  bytes_to_bits(const BYTES &);
    BYTES bits_to_bytes(const BITS &);

    void size_fit(BITS &bits, UINT32 size_of_bit);
    void size_fit(BYTES &bytes, UINT32 size_of_bit);

    BYTES pattern_fit(const BYTES &pattern, UINT32 size_of_byte);
}

#endif /* BYTES_BITS_H_ */
