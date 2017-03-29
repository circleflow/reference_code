
#include "test_helper.h"

#include "port.h"
#include "port_impl.h"
#include "type_ext.h"
#include "init.h"
#include "device.h"

#include "utility/export/error.h"
#include "utility/export/timer.h"
using namespace UTILITY;

#include "test_helper.h"

#include "hal_port_stub.hpp"
#include "mock_notify.h"
#include "mock_notify_stub.hpp"
#include "timer_mock_stub.hpp"

typedef DEVICE::CONFIG::PAIR_PROFILE PAIR_PROFILE;
typedef DEVICE::CONFIG::PAIR_PROFILE::PORT_PAIR PORT_PAIR;
typedef DEVICE::CONFIG::LATENCY_PROFILE LATENCY_PROFILE;

#define RJ45 PORT::ITF_RJ45
#define SFP  PORT::ITF_SFP

static PAIR_PROFILE gen_pair_profile (const string &name, UINT8 unit, PORT_PAIR pair)
{
    PAIR_PROFILE profile;
    profile.name = name;
    profile.unit = unit;
    profile.port_pair.push_back(pair);

    return profile;
}

static PAIR_PROFILE gen_pair_profile (const string &name, UINT8 unit, PORT_PAIR pair_0, PORT_PAIR pair_1)
{
    PAIR_PROFILE profile = gen_pair_profile(name, unit, pair_0);
    profile.port_pair.push_back(pair_1);

    return profile;
}

static PORT_PAIR gen_pair(PORT::INTERFACE itf, UINT8 engine, UINT8 front)
{
    PORT_PAIR p;
    p.phy_itf = itf;
    p.engine = engine;
    p.front = front;

    return p;
}

class PORT_test : public ::testing::Test {
 protected:

    bool check_mock;

    vector<PAIR_PROFILE> pair_v;
    vector<LATENCY_PROFILE> latency_v;

    int cac_interval;

    virtual void SetUp()
    {
        check_mock = false;

        opmock_test_reset();

        cac_interval = 2;

        pair_v.push_back(gen_pair_profile("ge 0/0",  0, gen_pair(RJ45, 2, 3), gen_pair(SFP, 3, 2)));
        pair_v.push_back(gen_pair_profile("ge 0/1",  0, gen_pair(RJ45, 4, 5), gen_pair(SFP, 5, 4)));
        pair_v.push_back(gen_pair_profile("ge 0/2",  0, gen_pair(RJ45, 6, 7), gen_pair(SFP, 7, 6)));
        pair_v.push_back(gen_pair_profile("ge 0/3",  0, gen_pair(RJ45, 8, 9), gen_pair(SFP, 9, 8)));
        pair_v.push_back(gen_pair_profile("ge 1/0",  1, gen_pair(SFP, 3, 2),  gen_pair(RJ45, 2, 3)));
        pair_v.push_back(gen_pair_profile("ge 1/1",  1, gen_pair(SFP, 5, 4),  gen_pair(RJ45, 4, 5)));
        pair_v.push_back(gen_pair_profile("ge 1/2",  1, gen_pair(SFP, 7, 6),  gen_pair(RJ45, 6, 7)));
        pair_v.push_back(gen_pair_profile("ge 1/3",  1, gen_pair(SFP, 9, 8),  gen_pair(RJ45, 8, 9)));
        pair_v.push_back(gen_pair_profile("xe 0/0",  0, gen_pair(SFP, 12, 13)));
        pair_v.push_back(gen_pair_profile("xe 1/0",  1, gen_pair(SFP, 12, 13)));

        {
            LATENCY_PROFILE profile;
            profile.unit = 0;
            profile.port = 24;
            latency_v.push_back(profile);
            profile.unit = 1;
            latency_v.push_back(profile);
        }

#define MAX_FRAME_SIZE 2000

    }

    virtual void TearDown()
    {
        if(check_mock) {
            EXPECT_VERIFY_MOCK();
        }
    }

    void mock_cnt_init_notify()
    {
        for(UINT8 i=0; i<pair_v.size(); i++) {

            hal_port_cnt_clear_ExpectAndReturn(
                    pair_v[i].unit, pair_v[i].port_pair[0].front,
                    cmp_byte, cmp_byte);

            hal_port_cnt_clear_ExpectAndReturn(
                    pair_v[i].unit, pair_v[i].port_pair[0].engine,
                    cmp_byte, cmp_byte);
        }
    }

    void mock_rate_get_cnt(TRX_CNT cnt, bool step)
    {
        for(UINT8 i=0; i<pair_v.size(); i++) {

            hal_port_cnt_get_ExpectAndReturn(
                    pair_v[i].unit, pair_v[i].port_pair[0].front,
                    cnt, cmp_byte, cmp_byte);

#define CNT_STEP {{2,128}, {4, 256}}
            if(step) {
                for(int i=0; i<cac_interval; i++) {
                    cnt += CNT_STEP;
                }
            }

        }
    }
};

bool CIRCLE_FLOW::DEVICE::is_init_done()
{
    return true;
}

ostream & operator << (ostream & os, const PORT::MODE &mode)
{
    os << "DUPLEX_SPEED:" << mode.ds << ", pause: " << mode.pause;
    return os;
}

static HAL_LINK_CB link_cb = 0;
static void link_callback(HAL_LINK_CB cb, int calls)
{
    link_cb = cb;

    EXPECT_TRUE(calls == 1);
}

static TIMER::CLIENT_OP timer_op;
void timer_construct_cb(TIMER *, int )
{  /*do nothing*/  }

void timer_set_op_cb(const TIMER::CLIENT_OP &op, int)
{
    timer_op = op;
}

TEST_F(PORT_test, basic)
{
    check_mock = true;

    PORT::DS_SET ds_set;
    ds_set.insert(PORT::FD_100MB);
    ds_set.insert(PORT::FD_1000MB);
    PORT::ABILITY ability;
    ability.ds_set = ds_set;
    ability.auto_nego = false;
    ability.pause = false;

    PORT::MODE mode;
    mode.ds = PORT::FD_1000MB;
    mode.pause = false;

    for(UINT8 i=0; i<pair_v.size(); i++) {

        hal_port_ability_get_ExpectAndReturn(
                pair_v[i].unit, pair_v[i].port_pair[0].front, ability,
                cmp_byte, cmp_byte);

        hal_port_forced_ExpectAndReturn(
                pair_v[i].unit, pair_v[i].port_pair[0].front, mode,
                cmp_byte, cmp_byte, cmp_type<PORT::MODE>);
    }

    for(UINT8 i=0; i<latency_v.size(); i++) {
        hal_port_ability_get_ExpectAndReturn(
                latency_v[i].unit, latency_v[i].port, ability,
                cmp_byte, cmp_byte);

        hal_port_forced_ExpectAndReturn(
                latency_v[i].unit, latency_v[i].port, mode,
                cmp_byte, cmp_byte, cmp_type<PORT::MODE>);

    }

    hal_port_link_callback_register_MockWithCallback(link_callback);

    timer_construct_MockWithCallback(timer_construct_cb);
    timer_set_op_MockWithCallback(timer_set_op_cb);
    timer_start_ExpectAndReturn (0, true, 0, cmp_type<bool>);

    mock_cnt_init_notify();

    hal_port_max_frame_size_set_ExpectAndReturn(MAX_FRAME_SIZE, cmp_type<UINT32>);

    CIRCLE_FLOW::port_init(pair_v, latency_v, MAX_FRAME_SIZE);

    //port all
    {
        PORT_NAME_SET port_all_actual=PORT::get_port_all();

        PORT_NAME_SET port_all_expect;
        for(UINT8 i=0; i<pair_v.size(); i++) {
            port_all_expect.insert(pair_v[i].name);
        }

        EXPECT_EQ(port_all_actual, port_all_expect);
    }

    //itf set
    {
        PORT::ITF_SET itfs_actual;
        itfs_actual = PORT::get_itf_set("ge 0/1");

        PORT::ITF_SET itfs_expect;
        itfs_expect.insert(RJ45);
        itfs_expect.insert(SFP);

        EXPECT_EQ(itfs_actual, itfs_expect);

        itfs_actual = PORT::get_itf_set("xe 1/0");
        itfs_expect.clear();
        itfs_expect.insert(SFP);
        EXPECT_EQ(itfs_actual, itfs_expect);
    }

    //itf current
    {
        EXPECT_EQ(RJ45, PORT::get_itf("ge 0/2"));
        EXPECT_EQ(SFP, PORT::get_itf("ge 1/2"));
    }

    //get front/engine
    {
        EXPECT_EQ(PORT_ID(0, 7), PORT_IMPL::get_front("ge 0/2"));
        EXPECT_EQ(PORT_ID(0, 6), PORT_IMPL::get_engine("ge 0/2"));
        EXPECT_EQ(PORT_ID(1, 6), PORT_IMPL::get_front("ge 1/2"));
        EXPECT_EQ(PORT_ID(1, 7), PORT_IMPL::get_engine("ge 1/2"));
    }
}

static void mock_link_notify(UINT8 instance, const PORT_NAME &port, bool link)
{
    link_notify_ExpectAndReturn (instance, port, link,
                                 cmp_byte, cmp_type<PORT_NAME>, cmp_type<bool>);
}

static void mock_port_status(UINT8 unit, UINT8 port, bool link)
{
    PORT::STATUS status;
    status.link = link;

    hal_port_status_get_ExpectAndReturn (unit, port, status, cmp_byte, cmp_byte);
}

TEST_F(PORT_test, link_notify)
{
    check_mock = true;

    EXPECT_TRUE(link_cb != 0);

    //empty notify list
    link_cb(0, 3, true);

    //single notify
    PORT_NAME_SET ports;
    ports.insert("ge 0/0");

    mock_port_status(0, 3, true);
    mock_link_notify(1, "ge 0/0", true);

    shared_ptr<LINK_CB> notify(new LINK_CB);
    *notify = bind(link_notify, 1, placeholders::_1, placeholders::_2);
    link_notify_bind(notify,ports);

    //link change
    mock_link_notify(1, "ge 0/0", false);
    link_cb(0, 3, false);

    //two notify on same port
    mock_port_status(0, 3, false);
    mock_link_notify(2, "ge 0/0", false);

    shared_ptr<LINK_CB> *notify_2 = new shared_ptr<LINK_CB>;
    notify_2->reset(new LINK_CB);
    *(*notify_2) = bind(link_notify, 2, placeholders::_1, placeholders::_2);
    link_notify_bind(*notify_2, ports);

    mock_link_notify(1, "ge 0/0", true);
    mock_link_notify(2, "ge 0/0", true);
    link_cb(0, 3, true);

    //same notify cb on different ports
    ports.clear();
    ports.insert("ge 1/0");
    ports.insert("xe 0/0");

    mock_port_status(1, 2,  false);
    mock_link_notify(3, "ge 1/0", false);
    mock_port_status(0, 13, false);
    mock_link_notify(3, "xe 0/0", false);

    shared_ptr<LINK_CB> notify_3(new LINK_CB);
    *notify_3 = bind(link_notify, 3, placeholders::_1, placeholders::_2);
    link_notify_bind(notify_3,ports);

    mock_link_notify(1, "ge 0/0", false);
    mock_link_notify(2, "ge 0/0", false);
    link_cb(0, 3, false);

    mock_link_notify(3, "ge 1/0", true);
    link_cb(1, 2, true);

    mock_link_notify(3, "xe 0/0", true);
    link_cb(0, 13, true);

    //unbind
    delete notify_2;
    mock_link_notify(1, "ge 0/0", true);
    link_cb(0, 3, true);

    //none-front port case
    link_cb(0, 2, false);
}

static void mock_pair_notify(UINT8 instance, const PORT_NAME &port, PORT_ID front, PORT_ID engine)
{
    pair_notify_ExpectAndReturn (instance, port, front, engine,
                                 cmp_byte, cmp_type<PORT_NAME>, cmp_type<PORT_ID>, cmp_type<PORT_ID>);
}

static void mock_cnt_reset_notify(PORT_ID front, PORT_ID engine)
{
    //for rate cac
    hal_port_cnt_clear_ExpectAndReturn(front.unit, front.index, cmp_byte, cmp_byte);
    hal_port_cnt_clear_ExpectAndReturn(engine.unit, engine.index, cmp_byte, cmp_byte);
}

TEST_F(PORT_test, pair_notify)
{
    check_mock = true;

    //single notify
    PORT_NAME_SET ports;
    ports.insert("ge 0/0");
    mock_pair_notify(1, "ge 0/0", PORT_ID(0,3), PORT_ID(0,2));

    shared_ptr<PAIR_CB> notify(new PAIR_CB);
    *notify = bind(pair_notify, 1, placeholders::_1, placeholders::_2, placeholders::_3);
    pair_notify_bind(notify,ports);

    mock_pair_notify(1, "ge 0/0", PORT_ID(0,2), PORT_ID(0,3));
    mock_cnt_reset_notify(PORT_ID(0,2), PORT_ID(0,3));
    PORT::set_itf("ge 0/0", SFP);

    //two notify on same port
    mock_pair_notify(2, "ge 0/0", PORT_ID(0,2), PORT_ID(0,3));

    shared_ptr<PAIR_CB> *notify_2 = new shared_ptr<PAIR_CB>;
    notify_2->reset(new PAIR_CB);
    *(*notify_2) = bind(pair_notify, 2, placeholders::_1, placeholders::_2, placeholders::_3);
    pair_notify_bind(*notify_2, ports);

    //same notify cb on different ports
    ports.clear();
    ports.insert("ge 1/0");
    ports.insert("ge 1/1");
    mock_pair_notify(3, "ge 1/0", PORT_ID(1,2), PORT_ID(1,3));
    mock_pair_notify(3, "ge 1/1", PORT_ID(1,4), PORT_ID(1,5));

    shared_ptr<PAIR_CB> notify_3(new PAIR_CB);
    *notify_3 = bind(pair_notify, 3, placeholders::_1, placeholders::_2, placeholders::_3);
    pair_notify_bind(notify_3, ports);

    mock_pair_notify(2, "ge 0/0", PORT_ID(0,3), PORT_ID(0,2));
    mock_pair_notify(1, "ge 0/0", PORT_ID(0,3), PORT_ID(0,2));
    mock_cnt_reset_notify(PORT_ID(0,3), PORT_ID(0,2));
    PORT::set_itf("ge 0/0", RJ45);

    //unbind
    delete notify_2;

    mock_pair_notify(1, "ge 0/0", PORT_ID(0,2), PORT_ID(0,3));
    mock_cnt_reset_notify(PORT_ID(0,2), PORT_ID(0,3));
    PORT::set_itf("ge 0/0", SFP);

    mock_pair_notify(3, "ge 1/0", PORT_ID(1,3), PORT_ID(1,2));
    mock_cnt_reset_notify(PORT_ID(1,3), PORT_ID(1,2));
    PORT::set_itf("ge 1/0", RJ45);

    //reset to default, for next case
    mock_pair_notify(1, "ge 0/0", PORT_ID(0,3), PORT_ID(0,2));
    mock_cnt_reset_notify(PORT_ID(0,3), PORT_ID(0,2));
    PORT::set_itf("ge 0/0", RJ45);

    mock_pair_notify(3, "ge 1/0", PORT_ID(1,2), PORT_ID(1,3));
    mock_cnt_reset_notify(PORT_ID(1,2), PORT_ID(1,3));
    PORT::set_itf("ge 1/0", SFP);
}

TEST_F(PORT_test, error_case)
{
    check_mock = true;

    EXPECT_THROW(PORT::get_itf_set("ge 2/1"), EXP_ERROR);

}

#include <limits>
using std::numeric_limits;
namespace UNIT_TEST {
    template<class T>
    T max_value(const T &)
    {
        return numeric_limits<T>::max();
    }
}

using namespace UNIT_TEST;

TEST_F(PORT_test, counter)
{
    check_mock = true;

    {
        TRX_CNT cnt={{1,64},{2,128}};
        hal_port_cnt_get_ExpectAndReturn(0, 3, cnt, cmp_byte, cmp_byte);
        EXPECT_EQ(cnt, PORT::get_cnt("ge 0/0", false));

        //reset cnt
        hal_port_cnt_get_ExpectAndReturn(0, 3, cnt, cmp_byte, cmp_byte);
        EXPECT_EQ(cnt, PORT::get_cnt("ge 0/0", true));

        //no increament
        hal_port_cnt_get_ExpectAndReturn(0, 3, cnt, cmp_byte, cmp_byte);
        EXPECT_EQ(TRX_CNT(), PORT::get_cnt("ge 0/0", true));
    }

    //value revert
    {
        TRX_CNT cnt{{99,9900},{66,6600}};
        hal_port_cnt_get_ExpectAndReturn(0, 3, cnt, cmp_byte, cmp_byte);
        cnt -= {{1,64},{2,128}};
        EXPECT_EQ(cnt, PORT::get_cnt("ge 0/0", true));

        TRX_CNT cnt2{{88,8800},{55,5500}};
        hal_port_cnt_get_ExpectAndReturn(0, 3, cnt2, cmp_byte, cmp_byte);
        cnt += {{1,64},{2,128}};
        TRX_CNT max{{max_value(cnt.tx.pkt),max_value(cnt.tx.byte)},{max_value(cnt.rx.pkt),max_value(cnt.rx.byte)}};

        cnt = max - cnt + cnt2;

        TRX_CNT cnt3 = PORT::get_cnt("ge 0/0", false);
        EXPECT_EQ(cnt, cnt3);
    }

    //another port
    {
        TRX_CNT cnt={{33,3333},{22,2222}};
        hal_port_cnt_get_ExpectAndReturn(0, 13, cnt, cmp_byte, cmp_byte);
        EXPECT_EQ(cnt, PORT::get_cnt("xe 0/0", false));
    }
}


TEST_F(PORT_test, rate)
{
    check_mock = true;

    TRX_CNT cnt;
    EXPECT_EQ(cnt, PORT::get_rate("ge 0/0"));

    //first rate cac
    cnt = {{22,1288},{44,2576}};
    mock_rate_get_cnt(cnt,false);
    timer_op();

    TRX_CNT cnt_rate = cnt/cac_interval;
    EXPECT_EQ(cnt_rate, PORT::get_rate("ge 0/1"));
    EXPECT_EQ(cnt_rate, PORT::get_rate("ge 1/0"));

    //second cac, with same cnt value
    mock_rate_get_cnt(cnt,false);
    timer_op();

    cnt_rate = TRX_CNT();
    EXPECT_EQ(cnt_rate, PORT::get_rate("ge 0/1"));
    EXPECT_EQ(cnt_rate, PORT::get_rate("ge 1/0"));

    //third cac, with increased cnt value
    cnt = {{22,1288},{44,2576}};
    mock_rate_get_cnt(cnt,true);
    timer_op();

    cnt_rate = CNT_STEP;
    EXPECT_EQ(cnt_rate, PORT::get_rate("ge 0/1"));

    cnt_rate += CNT_STEP;
    EXPECT_EQ(cnt_rate, PORT::get_rate("ge 0/2"));
}
