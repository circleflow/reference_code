
#ifndef CIRCLE_FLOW_FIELD_CONCRETE_H_
#define CIRCLE_FLOW_FIELD_CONCRETE_H_

#include "field.h"
#include "field_block.h"
#include "field_seed.h"
#include "field_parser.h"

#include <list>
using std::list;

namespace CIRCLE_FLOW {

    //field with unchangeable size
    class FIELD_FIXED_SIZE : public FIELD {
    public:

        FIELD_FIXED_SIZE(const FIELD::NAME &name,
                         unsigned int size_of_bit,
                         const PARSER &parser= HEX_PARSER());
        FIELD_FIXED_SIZE(const FIELD_FIXED_SIZE &ref);
        ~FIELD_FIXED_SIZE();

        virtual FIELD * clone(void) const;

        using FIELD::operator=;

    protected:
        FIELD_FIXED_SIZE();
        FIELD_FIXED_SIZE & operator = (const FIELD_FIXED_SIZE &ref);
    };


    //field which can generate its value based on the specified cac method
    class FIELD_CACULATOR : public FIELD_FIXED_SIZE {
    public:
        typedef BYTES (*DATA_CAC) (const BYTES &bytes);
        typedef BYTES (*SIZE_CAC) (unsigned int size_of_bit);

        //algorithm of ip header checksum, fit into 16 bit of field
        static BYTES ip_hdr_chksum(const BYTES &bytes);
        //transform bit length into byte length, fit into 16 bit of field
        static BYTES len_of_byte_16(unsigned int size_of_bit);

        enum FILTER{
            WHITE_LIST,
            BLACK_LIST
        };

        FIELD_CACULATOR(const FIELD::NAME & name,
                        unsigned int size_of_bit,
                        const FIELD::INDEX &cac_fields,
                        FILTER filter,
                        DATA_CAC cac,
                        const PARSER &parser= HEX_PARSER());
        FIELD_CACULATOR(const FIELD::NAME & name,
                        unsigned int size_of_bit,
                        const FIELD::INDEX &cac_fields,
                        FILTER filter,
                        SIZE_CAC cac,
                        const PARSER &parser= HEX_PARSER());
        FIELD_CACULATOR(const FIELD_CACULATOR &ref);
        ~FIELD_CACULATOR();

        virtual FIELD * clone(void) const;

        //once any value assigned, caculation would stopped, until empty value assigned
        using FIELD::operator=;

    protected:
        FIELD_CACULATOR();
        FIELD_CACULATOR & operator = (const FIELD_CACULATOR &ref);
    };


    //field with limited option of values
    class FIELD_OPTION : public FIELD_FIXED_SIZE {
    public:
        FIELD_OPTION(const FIELD::NAME & name,
                     unsigned int size_of_bit,
                     const PARSER &parser= HEX_PARSER());
        FIELD_OPTION(const FIELD_OPTION &ref);
        ~FIELD_OPTION();

        virtual FIELD * clone(void) const;

        using FIELD::operator=;

        void option_add(const BYTES &option);
        bool is_known_option(void) const;

    protected:
        FIELD_OPTION();
        FIELD_OPTION & operator = (const FIELD_OPTION &ref);
    };


    //field which size is elastic depends on the length of given value assigned
    class FIELD_RESIZABLE : public FIELD {
    public:

        FIELD_RESIZABLE(const FIELD::NAME &name,
                        unsigned int size_of_byte=0,
                        const PARSER &parser= HEX_PARSER());
        FIELD_RESIZABLE(const FIELD_RESIZABLE &ref);
        ~FIELD_RESIZABLE();

        void resize(unsigned int size_of_byte);

        virtual FIELD * clone(void) const;

        using FIELD::operator=;

    protected:
        FIELD_RESIZABLE();
        FIELD_RESIZABLE & operator = (const FIELD_RESIZABLE &ref);
    };

}

#endif
