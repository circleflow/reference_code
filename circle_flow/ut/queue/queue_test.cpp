
#include "test_helper.h"

#include "queue.h"
#include "type_ext.h"

#include "utility/export/error.h"
#include "utility/export/smart_ptr_u.h"

#include "init.h"
#include "hal.h"
#include "hal_queue_stub.hpp"

#include "test_helper.h"

#include "hal_stub.hpp"


class QUEUE_test : public ::testing::Test {
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


const HAL_UNIT_SPECS & CIRCLE_FLOW::hal_unit_specs(UINT8 unit)
{
    (void) unit;

    static HAL_UNIT_SPECS specs;

    return specs;
}

const vector<UINT8> & CIRCLE_FLOW::hal_unit_all(void)
{
    static vector<UINT8> units;

    if(0 == units.size()) {
        for(UINT8 unit=0; unit<2; unit++) {
            units.push_back(unit);
        }
    }

    return units;
}

const vector<UINT8> & CIRCLE_FLOW::hal_port_e_all(UINT8 unit)
{
    (void) unit;

    static vector<UINT8> ports;

    if(0 == ports.size()) {
        ports.push_back(2);
        ports.push_back(3);
        ports.push_back(4);
        ports.push_back(5);
    }

    return ports;
}

static UINT8 get_num_of_q_cb(UINT8 unit, UINT8 port, int calls)
{
    (void) unit;
    (void) calls;

    if( 5 == port ) return 8;

    return 4;
}

static vector< shared_ptr<QUEUE> > v_queue;
void q_alloc(UINT8 unit, UINT8 port, UINT8 num_of_q)
{
    for(UINT8 i=0; i<num_of_q; i++) {
        QUEUE *q = new QUEUE(unit, port);
        v_queue.push_back( shared_ptr<QUEUE> (q));
    }
}

void q_free_all(void)
{
    v_queue.clear();
}

TEST_F(QUEUE_test, basic)
{
    hal_q_num_MockWithCallback(get_num_of_q_cb);

    queue_init();

    for(UINT8 repeat=0; repeat<3; repeat++) {

        for(UINT8 unit=0; unit<2; unit++) {

            q_alloc(unit, 5, 8);
            EXPECT_THROW(q_alloc(unit, 5, 1), EXP_ERROR);

            q_alloc(unit, 2, 4);
            EXPECT_THROW(q_alloc(unit, 2, 1), EXP_ERROR);
        }

        q_free_all();
    }
}

bool operator == (const TOKEN_REFRESH &lhs, const TOKEN_REFRESH &rhs) {
    return (lhs.min==rhs.min) && (lhs.max==rhs.max);
}
bool operator != (const TOKEN_REFRESH &lhs, const TOKEN_REFRESH &rhs) {
    return ! (operator == (lhs, rhs));
}

ostream & operator << (ostream & os, const TOKEN_REFRESH &token)
{
    os << "max:" << token.max << ", min: " << token.min;
    return os;
}

static void mock_create_shaper(UINT8 unit, UINT8 port, RATE rate)
{
    UINT32 kbps = 0;

    switch(rate.type)
    {
        case RATE::KBITS_PER_SECOND:
            kbps = rate.value;
            break;
        case RATE::MBITS_PER_SECOND:
            kbps = rate.value * 1024;
            break;
        case RATE::GBITS_PER_SECOND:
            kbps = rate.value * 1024 * 1024;
            break;
        default:
            ERROR("invalid type");
    }

    hal_q_create_shaper_ExpectAndReturn (unit, port, 0, kbps, kbps,
                                     cmp_byte, cmp_byte, 0, cmp_int, cmp_int);

    TOKEN_REFRESH token(rate.value,rate.value/2);

    hal_q_get_shaper_ExpectAndReturn (unit, port, 0, token,
                                  cmp_byte, cmp_byte, 0);
}

void mock_stop_shaper(UINT8 unit, UINT8 port, RATE rate)
{

    hal_q_set_shaper_ExpectAndReturn (unit, port, 0, TOKEN_REFRESH(),
                                  cmp_byte, cmp_byte, 0, cmp_type<TOKEN_REFRESH>);
}

void mock_start_shaper(UINT8 unit, UINT8 port, RATE rate)
{
    TOKEN_REFRESH token(rate.value,rate.value/2);

    hal_q_set_shaper_ExpectAndReturn (unit, port, 0, token,
                                  cmp_byte, cmp_byte, 0, cmp_type<TOKEN_REFRESH>);
}

TEST_F(QUEUE_test, shaper)
{
    check_mock = true;

    QUEUE q1(0,2);
    RATE rate1(RATE::KBITS_PER_SECOND, 1000);

    mock_create_shaper(0, 2, rate1);
    q1.shaper(rate1);

    mock_stop_shaper(0, 2, rate1);
    q1.stop();

    QUEUE q2(1,5);
    RATE rate2(RATE::MBITS_PER_SECOND, 2);
    mock_create_shaper(1, 5, rate2);
    q2.shaper(rate2);

    mock_stop_shaper(1, 5, rate2);
    q2.stop();

    mock_start_shaper(0, 2, rate1);
    q1.start();

    mock_start_shaper(1, 5, rate2);
    q2.start();

    {
        QUEUE q3(0,3,0);
        EXPECT_THROW(QUEUE(0,3,0), EXP_ERROR);
    }

    QUEUE q4(0,3,0);
    EXPECT_THROW(QUEUE(0,3,4), EXP_ERROR);
}
