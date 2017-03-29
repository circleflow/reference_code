
#include "test_helper.h"
#include "flow/flow_track.h"

#include "utility/export/error.h"
#include "utility/export/_dependency.h"
using namespace UTILITY;

static
void terminate_mock (const char *file, int line, const char *str);

class FLOW_TRACK_test : public ::testing::Test {
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

//manual stub
#include "utility/export/random_u.h"
UINT32 rand_u32;
RAND::operator UINT32(void)
{
    return rand_u32;
}

RAND::operator UINT8(void)
{
    return 0xff;
}

static
string bytes_dump(const BYTES &, UINT8 )
{
    return string();
}

//test case
TEST_F(FLOW_TRACK_test, basic)
{
    rand_u32 = 0x01234567;

    FLOW_TRACK track;

    UINT8 data [] = {0x12, 0x34, 0x56, 0x70};
    EXPECT_EQ(BYTES(data, data+4), track.flow_bytes());

    UINT8 mask [] = {0xff, 0xff, 0xff, 0xf0};
    QUAL::MATCH match;
    match.value = BYTES(data, data+4);
    match.mask  = BYTES(mask, mask+4);
    EXPECT_EQ(match, track.flow_match());

    data[3] = 0x79;
    EXPECT_EQ(BYTES(data, data+4), track.sn_bytes(1));

    mask[3] = 0xf8;
    data[3] = 0x78;
    match.value = BYTES(data, data+4);
    match.mask  = BYTES(mask, mask+4);
    EXPECT_EQ(match, track.sn_match());

    EXPECT_EQ(0, track.extract_sn(BYTES(data, data+4)));
    data[3] = 0x7a;
    EXPECT_EQ(2, track.extract_sn(BYTES(data, data+4)));

    //test second instance
    {
        rand_u32 = 0x01122334;
        FLOW_TRACK track_2;

        UINT8 data [] = {0x11, 0x22, 0x33, 0x40};
        EXPECT_EQ(BYTES(data, data+4), track_2.flow_bytes());

        //automaticlly choose adjacent id
        FLOW_TRACK track_3;
        data[3] = 0x50;
        EXPECT_EQ(BYTES(data, data+4), track_3.flow_bytes());
    }

    //no error if object released
    FLOW_TRACK();
}

