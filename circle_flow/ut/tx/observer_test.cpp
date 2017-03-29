
#include "test_helper.h"
#include "mock_pkt_cb_stub.hpp"

#include "tx/pkt_helper.h"
#include "tx/observer.h"

#include "utility/export/_dependency.h"
using namespace UTILITY;

static
void terminate_mock (const char *file, int line, const char *str);

class OBSERVER_test : public ::testing::Test {
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

TEST_F(OBSERVER_test, basic)
{
    UINT8 unit = 0;
    UINT16 vid = 100;

    //add
    add_observer(unit, vid, bind(pkt_cb, placeholders::_1, placeholders::_2, placeholders::_3));

    //dispatch
    UINT32 time_stamp=0xffeeddcc;

    UINT8 data[] = {0x00,0x01,0x02,0x03,0x04,0x05,
                    0x00,0x11,0x22,0x33,0x44,0x55,
                    0x08,0x00};

    PACKET src(data, data+14);
    insert_vtag(src, src, 100);
    src.insert(src.end(), 60, 0x33);

    PACKET dst;
    remove_otag(src, dst);

    pkt_cb_ExpectAndReturn(unit, dst, time_stamp, cmp_byte, cmp_container<PACKET>, cmp_int);
    observer_dispatch(unit, src, time_stamp);

    //remove
    remove_observer(unit, vid);
    EXPECT_THROW(remove_observer(unit, vid), EXP_ERROR);

}
