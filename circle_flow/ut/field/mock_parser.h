
#ifndef MOCK_PARSER_H_
#define MOCK_PARSER_H_

#include "field_parser.h"

static TEXT  mock_parser_text;
static BYTES mock_parser_bytes;
class PARSER_MOCK : public FIELD::PARSER {
public:
    BYTES text_to_bytes(const TEXT &text) const
    {
        (void) text;
        return mock_parser_bytes;
    }

    TEXT bytes_to_text(const BYTES &bytes) const
    {
        (void) bytes;
        return mock_parser_text;
    }

    FIELD::PARSER * clone() const
    {
        return new PARSER_MOCK(*this);
    }
};



#endif
