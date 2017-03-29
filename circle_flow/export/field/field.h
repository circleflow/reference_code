
#ifndef CIRCLE_FLOW_FIELD_H_
#define CIRCLE_FLOW_FIELD_H_

#include "circle_flow/export/type.h"

#include <string>
using std::string;
#include <vector>
using std::vector;

namespace CIRCLE_FLOW {

    class FIELD {
    public:

        virtual ~FIELD();

        void operator = (const BYTES &bytes);
        void operator = (const TEXT &text);

        void pattern(const BYTES &bytes);
        void pattern(const TEXT &text);

        operator const BYTES & () const;
        operator const TEXT &  () const;

        unsigned int size_of_bit(void) const;

        virtual FIELD * clone(void) const = 0;

        /////////////////// relevant type //////////////
        typedef string NAME;    //name of field
        class IMPL;

        /*multiple value(incremental, decreased, random), detail refer field_seed.h
          once a seed assigned, the specific value get hidden*/
        class SEED;
        class STEP;     //detail in field_seed.h
        class RANDOM;   //detail in field_seed.h

        class INDEX;    //index of field within nested field blocks
        class PARSER;   //text <--> bytes

        NAME name() const;
        PARSER & parser();

        void operator = (const SEED &seed);

    protected:
        FIELD();
        FIELD(const FIELD &ref);
        FIELD & operator = (const FIELD &ref);

        friend class IMPL;
        IMPL *pimpl;
    };


    class FIELD::INDEX : public  vector<NAME> {
    public:
        INDEX(const char * str=0);
        INDEX(const NAME &);
        INDEX(vector<NAME>::const_iterator first,
              vector<NAME>::const_iterator last);

        using vector<NAME>::operator=;
    };
}


#endif
