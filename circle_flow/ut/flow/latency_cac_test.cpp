
#include "test_helper.h"
#include "flow/latency_cac.cpp"
#include "error.h"

class LATENCY_CAC_test : public ::testing::Test {
 protected:

    bool check_mock;


    virtual void SetUp()
    {
        check_mock = false;

        opmock_test_reset();

    }

    virtual void TearDown()
    {
        if(check_mock) {
            EXPECT_VERIFY_MOCK();
        }
    }
};

//manual stub
#include "flow/flow_track.h"
UINT8 FLOW_TRACK::max_sn()
{
    return 7;
}

#include "timer_mock_stub.hpp"
#include "pkt_gen_mock_stub.hpp"

static TIMER::CLIENT_OP timer_op;
static TIMER *timer_ptr;
void timer_construct_cb(TIMER * timer, int)
{
    timer_ptr = timer;
}

void timer_set_op_cb(const TIMER::CLIENT_OP &op, int)
{
    timer_op = op;
}

//test case
TEST_F(LATENCY_CAC_test, basic)
{
    check_mock = true;

    //lc construct
    timer_construct_MockWithCallback(timer_construct_cb);
    timer_set_op_MockWithCallback(timer_set_op_cb);
    LATENCY_CAC lc;

    //start
    timer_start_ExpectAndReturn (0, false, 0, cmp_type<bool>);
    lc.start(bind(gen_op, placeholders::_1));

    //1st time out
    UINT8 data [] = {0,1,2,3,4,5,6,7};
    vector<UINT8> sn(data, data+8);
    gen_op_ExpectAndReturn(sn, cmp_container< vector<UINT8> >);
    timer_start_ExpectAndReturn (0, false, 0, cmp_type<bool>);
    timer_op();

    //fill up tx timestamp
    {
        UINT32 ts[] = {0x33330000, 0x33330100, 0x33330200, 0x33330300,
                       0x33330400, 0x33330500, 0x33330600, 0x33330700};

        for(UINT8 i=0; i<(sizeof(ts)/sizeof(ts[0])); i++) {
            lc.set_tx_timestamp(i,ts[i]);
        }
    }

    //2nd time out
    timer_start_ExpectAndReturn (0, false, 0, cmp_type<bool>);
    timer_op();

    //fill up rx timestamp
    {
        UINT32 ts[] = {0x333300a0, 0x33330190, 0x33330280, 0x33330390,
                       0x33330480, 0x333305a0, 0x33330688, 0x33330798};

        for(UINT8 i=0; i<(sizeof(ts)/sizeof(ts[0])); i++) {
            lc.set_rx_timestamp(i,ts[i]);
        }
    }

    EXPECT_EQ(LATENCY_INVALID, lc.get_latency());

    //3nd time out
    timer_start_ExpectAndReturn (0, false, 0, cmp_type<bool>);
    timer_op();

    EXPECT_EQ(0x90, lc.get_latency());

    //4th time out
    gen_op_ExpectAndReturn(sn, cmp_container< vector<UINT8> >);
    timer_start_ExpectAndReturn (0, false, 0, cmp_type<bool>);
    timer_op();

    //lc destruct
    timer_destruct_ExpectAndReturn (timer_ptr, cmp_ptr);
}

TEST_F(LATENCY_CAC_test, cac)
{
    //miss
    {
        UINT32 tx[] = {0x30003333, 0x31003333, 0x32003333, 0x33003333,
                       0x34003333, 0x35003333, 0x36003333, 0x37003333};
        UINT32 rx[] = {0x30803333, 0x31883333, 0x32783333, TS_INVALID,
                       0x34803333, 0x35883333, 0x36783333, 0x37803333};

        EXPECT_EQ(0x00800000, cac_latency(vector<UINT32>(tx, tx+8), vector<UINT32>(rx, rx+8)));
    }

    //miss and turn over
    {
        UINT32 tx[] = {0xfffff900, 0xfffffa00, 0xfffffb00, TS_INVALID,
                       0xfffffd00, 0xfffffe00, 0xffffff00, 0x00000001};
        UINT32 rx[] = {0xfffff9a0, 0xfffffab0, 0xfffffb90, 0xfffffca0,
                       0xfffffda0, 0xfffffea0, 0x00000010, 0x00000031};

        EXPECT_EQ(0x000000a0, cac_latency(vector<UINT32>(tx, tx+8), vector<UINT32>(rx, rx+8)));

    }
}
