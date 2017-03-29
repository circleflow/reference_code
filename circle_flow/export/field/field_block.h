
#ifndef CIRCLE_FLOW_FIELD_BLOCK_H_
#define CIRCLE_FLOW_FIELD_BLOCK_H_

#include "field.h"

namespace CIRCLE_FLOW {

    class FIELD_BLOCK : public FIELD {
    public:
        FIELD_BLOCK(const FIELD::NAME &name="");
        FIELD_BLOCK(const FIELD_BLOCK &ref);
        ~FIELD_BLOCK();

        void rename(const FIELD::NAME &name);

        virtual FIELD * clone(void) const;
        void reset(const FIELD_BLOCK &fb=FIELD_BLOCK());

        void append (const FIELD &f);
        void insert (const FIELD::INDEX &idx, const FIELD &f);
        void remove (const FIELD::INDEX &idx);
        void extend (const FIELD::INDEX &idx, const FIELD_BLOCK &fb);

        FIELD_BLOCK & block (const FIELD::INDEX  &idx);
        const FIELD_BLOCK & block (const FIELD::INDEX  &idx)  const;

        FIELD & field (const FIELD::INDEX  &idx);
        const FIELD & field (const FIELD::INDEX  &idx)  const;

        int offset_of_bit(const FIELD::INDEX &idx) const;

        operator const PACKETS & () const;

        string dump() const;

        using FIELD::operator=;

    protected:
        FIELD_BLOCK & operator = (const FIELD_BLOCK &ref);

    };

}

#endif /* EXPORT_FIELD_FIELD_BLOCK_H_ */
