
#include "test_helper.h"

#include "utility/export/error.h"
#include "utility/export/_dependency.h"
using namespace UTILITY;

#include "cpu.h"
#include "type_ext.h"
#include "device.h"
#include "init.h"

#include "utility/export/smart_ptr_u.h"

#include "test_helper.h"

#include "cpu_hal_stub.hpp"
#include "queue_stub.hpp"
#include "hal_stub.hpp"
#include "mock_pkt_rx_stub.hpp"

#include <set>
using std::set;

static
void terminate_mock (const char *file, int line, const char *str);

class CPU_test : public ::testing::Test {
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

static const UINT8 NUM_OF_CPU_Q = 48;
static const UINT8 _units[] = {0, 1};
static const UINT8 _ports[] = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21};

static void mock_cpu_init(void)
{
    static vector<UINT8> units(_units, _units+2);
    hal_unit_all_ExpectAndReturn (units);

    static vector<UINT8> ports(_ports, _ports+20);
    hal_port_e_all_ExpectAndReturn (0, ports, 0);

    static HAL_UNIT_SPECS specs;
    specs.cpu_port_id = 0;
    hal_unit_specs_ExpectAndReturn (0, specs, 0);

    QUEUE_Mock::num_of_q_ExpectAndReturn(0, 0, NUM_OF_CPU_Q, cmp_byte, cmp_byte);

    hal_unit_all_ExpectAndReturn (units);
}

static hal_cpu_rx_cb rx_cb;
void hal_cpu_init_cb( hal_cpu_rx_cb cb, int calls)
{
    rx_cb = cb;
    EXPECT_EQ(1, calls);
}

static void mock_hal_init(void)
{
    hal_cpu_init_MockWithCallback(hal_cpu_init_cb);
}

TEST_F(CPU_test, init_basic)
{
    check_mock = true;

    mock_cpu_init();

    mock_hal_init();

    cpu_init();
}

static void mock_pkt_rx(UINT8 unit, const PACKET &pkt, UINT32 time_stamp)
{
    pkt_rx_ExpectAndReturn (unit, pkt, time_stamp,
            cmp_byte, cmp_container<PACKET>, cmp_int);
}

static void mock_pkt_rx_2(UINT8 unit, const PACKET &pkt, UINT32 time_stamp)
{
    pkt_rx_2_ExpectAndReturn (unit, pkt, time_stamp,
            cmp_byte, cmp_container<PACKET>, cmp_int);
}

static void mock_pkt_rx_3(UINT8 unit, const PACKET &pkt, UINT32 time_stamp)
{
    pkt_rx_3_ExpectAndReturn (unit, pkt, time_stamp,
            cmp_byte, cmp_container<PACKET>, cmp_int);
}

static void mock_pkt_rx_4(UINT8 unit, const PACKET &pkt, UINT32 time_stamp)
{
    pkt_rx_4_ExpectAndReturn (unit, pkt, time_stamp,
            cmp_byte, cmp_container<PACKET>, cmp_int);
}

TEST_F(CPU_test, observer_basic)
{
    check_mock = true;

    EXPECT_TRUE(rx_cb != 0);

    UINT8 unit=0;
    vector<UINT8> ports(1,2);

    CPU::RX *rx = new CPU::RX(unit, ports);
    EXPECT_EQ(unit, rx->get_unit());
    EXPECT_EQ(ports, rx->get_ports());

    UINT8 qid = rx->get_qid();
    EXPECT_TRUE( qid < NUM_OF_CPU_Q);

    PACKET pkt(100, 0x55);
    UINT32 time_stamp = 0x01020304;

    //bind
    rx->bind(bind(pkt_rx,placeholders::_1, placeholders::_2, placeholders::_3));
    mock_pkt_rx(unit, pkt, time_stamp);
    rx_cb(unit, qid, ports[0], pkt, time_stamp);

    //unbind
    rx->unbind();
    rx_cb(unit, qid, ports[0], pkt, time_stamp);

    //repeated bind
    rx->bind(bind(pkt_rx,placeholders::_1, placeholders::_2, placeholders::_3));
    mock_pkt_rx(unit, pkt, time_stamp);
    rx_cb(unit, qid, ports[0], pkt, time_stamp);

    rx->bind(bind(pkt_rx_2,placeholders::_1, placeholders::_2, placeholders::_3));

    mock_pkt_rx_2(unit, pkt, time_stamp);
    rx_cb(unit, qid, ports[0], pkt, time_stamp);

    //unbind by CPU::RX
    delete rx;

    //unbind
    rx_cb(unit, qid, ports[0], pkt, time_stamp);

}

TEST_F(CPU_test, basic_error)
{
    check_mock = true;

    EXPECT_THROW(CPU::RX rx(_units[0], vector<UINT8>(1,1)), EXP_ERROR);

    //EXPECT_THROW(CPU::RX rx(2, vector<UINT8>(1,2)), EXP_ERROR);

    vector< shared_ptr<CPU::RX> >v_rx;
    for(UINT8 i=0; i<NUM_OF_CPU_Q; i++) {
        UINT8 unit = _units[0];
        vector<UINT8> ports (_ports, _ports+10);
        v_rx.push_back(shared_ptr<CPU::RX>(new CPU::RX(unit, ports)));
    }

    //EXPECT_THROW(CPU::RX rx(_units[0], vector<UINT8>(_ports, _ports+1)), EXP_ERROR);

    CPU::RX rx(_units[1], vector<UINT8>(_ports, _ports+1));

    CPU::RX rx_1(_units[0], vector<UINT8>(_ports+10, _ports+11));
}

TEST_F(CPU_test, observer_two_unit)
{
    check_mock = true;

    vector<UINT8> ports(1,2);
    CPU::RX rx(_units[0], ports);
    CPU::RX rx_2(_units[1], ports);

    EXPECT_EQ(_units[0], rx.get_unit());
    EXPECT_EQ(_units[1], rx_2.get_unit());
    EXPECT_EQ(ports, rx.get_ports());
    EXPECT_EQ(ports, rx_2.get_ports());

    EXPECT_EQ(rx.get_qid(), rx_2.get_qid());

    PACKET pkt(100, 0x11);
    UINT32 time_stamp = 0x22222222;

    rx.bind(bind(pkt_rx,placeholders::_1, placeholders::_2, placeholders::_3));
    mock_pkt_rx(_units[0], pkt, time_stamp);
    rx_cb(_units[0], rx.get_qid(), ports[0], pkt, time_stamp);

    PACKET pkt_2(1000, 0x77);
    UINT32 time_stamp_2 = 0x88880000;
    rx_2.bind(bind(pkt_rx_2,placeholders::_1, placeholders::_2, placeholders::_3));
    mock_pkt_rx_2(_units[1], pkt_2, time_stamp_2);
    rx_cb(_units[1], rx_2.get_qid(), ports[0], pkt_2, time_stamp_2);
}

TEST_F(CPU_test, weight)
{
    check_mock = true;

    vector<UINT8> ports(_ports, _ports+3);
    CPU::RX rx(_units[0], ports);

    vector<UINT8> ports_2(_ports+3, _ports+6);
    CPU::RX rx_2(_units[0], ports_2);

    EXPECT_EQ(_units[0], rx.get_unit());
    EXPECT_EQ(_units[0], rx_2.get_unit());
    EXPECT_EQ(ports, rx.get_ports());
    EXPECT_EQ(ports_2, rx_2.get_ports());

    //2 queue weight compare
    EXPECT_EQ(rx.get_qid(), rx_2.get_qid());

    PACKET pkt(100, 0x11);
    UINT32 time_stamp = 0x22222222;

    rx.bind(bind(pkt_rx,placeholders::_1, placeholders::_2, placeholders::_3));
    mock_pkt_rx(_units[0], pkt, time_stamp);
    rx_cb(_units[0], rx.get_qid(), ports[0], pkt, time_stamp);

    PACKET pkt_2(1000, 0x77);
    UINT32 time_stamp_2 = 0x88880000;
    rx_2.bind(bind(pkt_rx_2,placeholders::_1, placeholders::_2, placeholders::_3));
    mock_pkt_rx_2(_units[0], pkt_2, time_stamp_2);
    rx_cb(_units[0], rx_2.get_qid(), ports_2[0], pkt_2, time_stamp_2);

    //3 queue weight compare
    vector<UINT8> ports_3(_ports+2, _ports+4);
    CPU::RX rx_3(_units[0], ports_3);

    vector<UINT8> ports_4(_ports+6, _ports+8);
    CPU::RX rx_4(_units[0], ports_4);

    EXPECT_EQ(_units[0], rx_3.get_unit());
    EXPECT_EQ(_units[0], rx_4.get_unit());
    EXPECT_EQ(ports_3, rx_3.get_ports());
    EXPECT_EQ(ports_4, rx_4.get_ports());

    EXPECT_NE(rx_2.get_qid(), rx_3.get_qid());
    EXPECT_NE(rx_3.get_qid(), rx_4.get_qid());

    EXPECT_EQ(rx.get_qid(), rx_4.get_qid());

    PACKET pkt_3(100, 0x83);
    UINT32 time_stamp_3 = 0x88888800;
    rx_3.bind(bind(pkt_rx_3,placeholders::_1, placeholders::_2, placeholders::_3));
    mock_pkt_rx_3(_units[0], pkt_3, time_stamp_3);
    rx_cb(_units[0], rx_3.get_qid(), ports_3[1], pkt_3, time_stamp_3);

    PACKET pkt_4(200, 0x78);
    UINT32 time_stamp_4 = 0x88888888;
    rx_4.bind(bind(pkt_rx_4,placeholders::_1, placeholders::_2, placeholders::_3));
    mock_pkt_rx_4(_units[0], pkt_4, time_stamp_4);
    rx_cb(_units[0], rx_4.get_qid(), ports_4[0], pkt_4, time_stamp_4);

}

