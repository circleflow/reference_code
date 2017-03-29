
#include "field_parser.h"
#include "type_ext.h"

#include "utility/export/error.h"

#include <stdio.h>
#include <sstream>
using std::istringstream;
using std::ostringstream;
#include <algorithm>
using std::replace;

FIELD::PARSER::PARSER()
{ }

FIELD::PARSER::~PARSER()
{ }

inline BYTES extract_bytes(istringstream &iss)
{
    BYTES bytes;

    while(!iss.eof() && !iss.fail()) {

        UINT32 u32;
        iss>>u32;

        ENSURE(u32<=0xff);

        bytes.push_back((UINT8)u32);
    }

    return bytes;
}

static char hex_delimiters[] = {':', '-'};

BYTES HEX_PARSER::text_to_bytes(const TEXT &_text) const
{
    // replace the delimiter with white space
    string text(_text);
    for(UINT8 i=0; i<sizeof(hex_delimiters); i++) {
        replace(text.begin(), text.end(), hex_delimiters[i], ' ');
    }

    istringstream iss(text);
    iss.setf (std::ios::hex , std::ios::basefield);

    return extract_bytes(iss);
}

TEXT HEX_PARSER::bytes_to_text(const BYTES &bytes) const
{
    //two ways to convert bytes to text: ostream vs printf
    //the latter is preferred for performance concern
#if 0
    ostringstream oss;

    BYTES::const_iterator last = bytes.end();
    for(BYTES::const_iterator it = bytes.begin(); it!=last; ) {

        oss.width(2);
        oss.fill('0');
        oss.setf (std::ios::hex , std::ios::basefield);
        oss.setf (std::ios::right , std::ios::adjustfield);

        UINT32 u32 = *it;
        oss<<u32;

        it++;

        if(it!=last) {
            oss<<hex_delimiters[0];
        }
    }

    return oss.str();
#else

    string text;

    UINT16 i=0;
    UINT16 max = bytes.size();

#define BYTES_NUM 8

    for(UINT16 num = max-i; num>=BYTES_NUM ; ) {
        char buff [BYTES_NUM*3+1];

        snprintf(buff, sizeof(buff), "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
                bytes[i],bytes[i+1],bytes[i+2],bytes[i+3],bytes[i+4],bytes[i+5],bytes[i+6],bytes[i+7]);
        text += buff;

        i+=BYTES_NUM;
        num = max-i;
        if(num>0) {
            text += ":";
        }
    }

    for(; i<max; i++){
        char buff[3+1];

        snprintf(buff, sizeof(buff),
                i==(max-1) ? "%02x":"%02x:",
                bytes[i]);

        text += buff;
    }

    return text;

#endif
}

FIELD::PARSER * HEX_PARSER::clone() const
{
    return new HEX_PARSER(*this);
}

static char dec_delimiters[] = {'.'};

BYTES DEC_PARSER::text_to_bytes(const TEXT &_text) const
{
    // replace the delimiter with blank
    string text(_text);
    for(UINT8 i=0; i<sizeof(dec_delimiters); i++) {
        replace(text.begin(), text.end(), dec_delimiters[i], ' ');
    }

    istringstream iss(text);
    iss.setf (std::ios::dec , std::ios::basefield);

    return extract_bytes(iss);
}

TEXT DEC_PARSER::bytes_to_text(const BYTES &bytes) const
{
    ostringstream oss;

    BYTES::const_iterator last = bytes.end();
    for(BYTES::const_iterator it = bytes.begin(); it!=last; ) {

        oss.width(3);
        oss.fill('0');
        oss.setf (std::ios::dec , std::ios::basefield);
        oss.setf (std::ios::right , std::ios::adjustfield);

        UINT32 u32 = *it;
        oss<<u32;

        it++;

        if(it!=last) {
            oss<<dec_delimiters[0];
        }
    }

    return oss.str();
}

FIELD::PARSER * DEC_PARSER::clone() const
{
    return new DEC_PARSER(*this);
}

