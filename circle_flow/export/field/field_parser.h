
#ifndef CIRCLE_FLOW_FIELD_PARSER_H_
#define CIRCLE_FLOW_FIELD_PARSER_H_

#include "field.h"

namespace CIRCLE_FLOW {

    class FIELD::PARSER {
    public:
        PARSER();
        virtual ~PARSER();
        virtual FIELD::PARSER * clone() const = 0;

        virtual BYTES text_to_bytes(const TEXT &text) const = 0;
        virtual TEXT  bytes_to_text(const BYTES &bytes) const = 0;
    };

    //support format as: "xx-xx", "xx:xx", "xx xx"
    class HEX_PARSER : public FIELD::PARSER{
    public:
        BYTES text_to_bytes(const TEXT &text) const;
        TEXT  bytes_to_text(const BYTES &bytes) const;

        FIELD::PARSER * clone() const;
    };

    //support format as: "xxx.xxx"
    class DEC_PARSER : public FIELD::PARSER{
    public:
        BYTES text_to_bytes(const TEXT &text) const;
        TEXT  bytes_to_text(const BYTES &bytes) const;

        FIELD::PARSER * clone() const;
    };

}

#endif /* FIELD_PARSER_H_ */
