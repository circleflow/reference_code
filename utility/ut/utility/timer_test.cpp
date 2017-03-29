
#include "test_helper.h"

#include "time_u.h"
#include "timer.h"
#include "trace.h"
using namespace UTILITY;

class TIMER_test : public ::testing::Test {
 protected:

    bool check_mock;

    virtual void SetUp()
    {
        check_mock = false;
        opmock_test_reset();

        //TRACE_FILTER::enable("timer");
    }

    virtual void TearDown()
    {
        if(check_mock) {
            EXPECT_VERIFY_MOCK();
        }
    }
};

static int cb_n;
void callback_op()
{
    cb_n ++;
}

TEST_F(TIMER_test, basic)
{
    cb_n = 0;

    TIMER timer;
    timer.set_op(callback_op);

    timer.start(100);
    EXPECT_TRUE(0 == cb_n);

    TIME::sleep_ms(200);
    EXPECT_TRUE(1 == cb_n);

    timer.start(100);
    EXPECT_TRUE(1 == cb_n);

    TIME::sleep_ms(200);
    EXPECT_TRUE(2 == cb_n);

}

#include <vector>
using std::vector;
#include "function_bind.h"
#include "stdio.h"

static vector<int> sn_log;
void callback_op_log(int sn)
{
    //printf(" callback: %d, %d \r\n", sn, TIME::get_elapsed_ms());
    sn_log.push_back(sn);
}

TEST_F(TIMER_test, sequence)
{
    //printf(" started:     %d \r\n", TIME::get_elapsed_ms());

    TIMER timer_3;
    timer_3.set_op(bind(callback_op_log,3));
    timer_3.start(300);

    TIMER timer_1;
    timer_1.set_op(bind(callback_op_log,1));
    timer_1.start(100);

    TIMER timer_2;
    timer_2.set_op(bind(callback_op_log,2));
    timer_2.start(200);

    TIMER timer_5;
    timer_5.set_op(bind(callback_op_log,5));
    timer_5.start(500);

    TIMER timer_4;
    timer_4.set_op(bind(callback_op_log,4));
    timer_4.start(400);

    TIME::sleep_ms(600);

    EXPECT_TRUE(5 == sn_log.size());

    for(int i=0; i<4; i++) {
        EXPECT_TRUE((i+1) == sn_log[i]);
    }
}


void callback_op_cnt(TIMER *timer)
{
    if (1==cb_n) {
        timer->stop();
    }

    cb_n--;
}

TEST_F(TIMER_test, periodical)
{
    cb_n=3;

    TIMER timer_1;
    timer_1.set_op(bind(callback_op_cnt, &timer_1));
    timer_1.start(100, true);

    TIME::sleep_ms(600);

    EXPECT_TRUE(0 == cb_n);
}

#include "mutex_u.h"
using namespace UTILITY;

struct MUTEX_1 {};
typedef AUTO_MUTEX<MUTEX_1> GUARD_T;
typedef TIMER_GUARD<GUARD_T::lock, GUARD_T::unlock> TIMER_G;
TEST_F(TIMER_test, instance_delete)
{
    GUARD_T::lock();

    sn_log.clear();

    TIMER_G timer_3;
    timer_3.set_op(bind(callback_op_log,3));
    timer_3.start(300);

    TIMER_G timer_1;
    timer_1.set_op(bind(callback_op_log,1));
    timer_1.start(100);

    {
        TIMER_G timer_2;
        timer_2.set_op(bind(callback_op_log,2));
        timer_2.start(200);
    }

    TIMER_G timer_5;
    timer_5.set_op(bind(callback_op_log,5));
    timer_5.start(500);

    {
        TIMER_G timer_4;
        timer_4.set_op(bind(callback_op_log,4));
        timer_4.start(400);
    }

    GUARD_T::unlock();

    TIME::sleep_ms(600);

    EXPECT_TRUE(3 == sn_log.size());

    EXPECT_TRUE(1 == sn_log[0]);
    EXPECT_TRUE(3 == sn_log[1]);
    EXPECT_TRUE(5 == sn_log[2]);
}

