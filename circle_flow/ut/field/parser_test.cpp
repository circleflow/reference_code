
#include "test_helper.h"

#include "type_ext.h"
#include "error.h"
#include "field_concrete.h"
#include "field_parser.h"
#include "test_helper.h"

#include "utility/export/error.h"
#include "utility/export/_dependency.h"
using namespace UTILITY;

static
void terminate_mock (const char *file, int line, const char *str);

class PARSER_test : public ::testing::Test {
 protected:

    bool check_mock;


    virtual void SetUp()
    {
        check_mock = false;

        opmock_test_reset();

        error_set_terminate(terminate_mock);

    }

    virtual void TearDown()
    {
        if(check_mock) {
            EXPECT_VERIFY_MOCK();
        }
    }
};

static
void terminate_mock (const char *file, int line, const char *str)
{
    ERROR("terminate called");
}

TEST_F(PARSER_test, hex_parser)
{
    HEX_PARSER hex;
    UINT8 data[] = {0x00,0x01,0x10,0x0a,0xa0,0xff};

    //"xx-xx"
    {
        BYTES bytes = hex.text_to_bytes(TEXT("00-01-10-0a-a0-ff"));
        EXPECT_EQ(BYTES(data, data+6), bytes);
        EXPECT_EQ(TEXT("00:01:10:0a:a0:ff"), hex.bytes_to_text(bytes));
    }

    //"xx:xx"
    {
        FIELD::PARSER *parser = hex.clone();
        EXPECT_EQ(BYTES(data, data+6), parser->text_to_bytes(TEXT("00:01:10:0a:a0:ff")));
    }

    //"xx xx"
    EXPECT_EQ(BYTES(data, data+6), hex.text_to_bytes(TEXT("00 01 10 0a a0 ff")));

    //abnormal case
    EXPECT_THROW(hex.text_to_bytes(TEXT("100")), EXP_ERROR);

    //big data
    {
        TEXT text("46:c0:00:28:7c:f7:00:00:01:02:05:aa:0a:08:10:05:e0:00:00:03:94:04:00:00:22:00:f8:fc:00:00:00:01:04:00:00:00:e1:00:00:01");
        BYTES bytes = hex.text_to_bytes(text);
        EXPECT_EQ(text, hex.bytes_to_text(bytes));
    }
}

TEST_F(PARSER_test, dec_parser)
{
    DEC_PARSER dec;
    UINT8 data[] = {0, 1, 10, 255};

    //xxx.xxx
    EXPECT_EQ(BYTES(data, data+4), dec.text_to_bytes(TEXT("0.1.10.255")));
    {
        FIELD::PARSER *parser = dec.clone();
        EXPECT_EQ(TEXT("000.001.010.255"), parser->bytes_to_text(BYTES(data, data+4)));
    }

    //abnormal case
    EXPECT_THROW(dec.text_to_bytes(TEXT("256")), EXP_ERROR);
}

