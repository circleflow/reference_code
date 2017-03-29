
#include "test_helper.h"

#include "tpid.h"
#include "type_ext.h"
#include "defs.h"

#include "port_impl_stub.hpp"
#include "hal_tpid_stub.hpp"

#include "utility/export/_dependency.h"
using namespace UTILITY;

static
void terminate_mock (const char *file, int line, const char *str);

static
UINT8 hal_tpid_max_id_cb(int )
{
    return 3;
}

class TPID_test : public ::testing::Test {
 protected:

    bool check_mock;

    PORT_ID ports[4];
    UINT16 tpids[2];


    virtual void SetUp()
    {
        check_mock = false;
        opmock_test_reset();

        ports[0].unit  = 0;
        ports[0].index = 2;

        ports[1].unit  = 0;
        ports[1].index = 4;

        ports[2].unit  = 1;
        ports[2].index = 2;

        ports[3].unit  = 1;
        ports[3].index = 6;

        tpids[0] = 0x8100;
        tpids[1] = 0x9100;

        //CIRCLE_FLOW::_trc_filter_verbose.enable();

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

static void mock_construct_unit_tpid(UINT8 unit, UINT8 index, UINT16 tpid)
{
    hal_tpid_unit_set_ExpectAndReturn(unit, index, tpid, cmp_byte, cmp_byte, cmp_short);
}

static void mock_construct_port_tpid(PORT_ID port, UINT8 bmp)
{
    hal_tpid_port_set_ExpectAndReturn(port.unit, port.index, bmp, cmp_byte, cmp_byte, cmp_byte);
}


TEST_F(TPID_test, same_tpid)
{
    check_mock = true;

    hal_tpid_max_id_MockWithCallback(hal_tpid_max_id_cb);

    mock_construct_unit_tpid(ports[0].unit, 0, tpids[0]);
    mock_construct_port_tpid(ports[0], 0x1);

    static OUTER_TPID ot_1 (ports[0], tpids[0]);

    //nothing to do with repeate
    OUTER_TPID ot_1_2 (ports[0], tpids[0]);

    mock_construct_port_tpid(ports[1], 0x1);
    static OUTER_TPID ot_2 (ports[1], tpids[0]);

    //nothing to do with repeate
    OUTER_TPID ot_2_2 (ports[1], tpids[0]);
}


TEST_F(TPID_test, different_tpid)
{
    check_mock = true;

    mock_construct_unit_tpid(ports[0].unit, 1, tpids[1]);
    mock_construct_port_tpid(ports[0], 0x3);
    OUTER_TPID ot_1(ports[0], tpids[1]);

    mock_construct_port_tpid(ports[1], 0x3);
    OUTER_TPID ot_2(ports[1], tpids[1]);

    //destruct
    mock_construct_port_tpid(ports[1], 0x1);
    mock_construct_port_tpid(ports[0], 0x1);
}

TEST_F(TPID_test, different_unit)
{
    check_mock = true;

    mock_construct_unit_tpid(ports[2].unit, 0, tpids[0]);
    mock_construct_port_tpid(ports[2], 0x1);
    static OUTER_TPID ot_1(ports[2], tpids[0]);

    mock_construct_port_tpid(ports[3], 0x1);
    static OUTER_TPID ot_2(ports[3], tpids[0]);

}

TEST_F(TPID_test, dimension)
{
    check_mock = true;

    mock_construct_unit_tpid(ports[0].unit, 2, tpids[1]);
    mock_construct_port_tpid(ports[0],0x5);
    OUTER_TPID *ot_1 = new OUTER_TPID(ports[0], tpids[1]);

    mock_construct_unit_tpid(ports[1].unit, 3, 0x3300);
    mock_construct_port_tpid(ports[1],0x9);
    OUTER_TPID *ot_1_2 = new OUTER_TPID(ports[1], 0x3300);

    //index recycle to 1
    mock_construct_unit_tpid(ports[1].unit, 1, 0x4400);
    mock_construct_port_tpid(ports[1],0xb);
    OUTER_TPID *ot_1_3 = new OUTER_TPID(ports[1], 0x4400);

    mock_construct_port_tpid(ports[0],0x7);
    OUTER_TPID *ot_1_4 = new OUTER_TPID(ports[0], 0x4400);

    EXPECT_THROW(OUTER_TPID(ports[0],0x5500), EXP_ERROR);

    mock_construct_unit_tpid(ports[2].unit, 1, tpids[1]);
    mock_construct_port_tpid(ports[2],0x3);
    OUTER_TPID *ot_2 = new OUTER_TPID(ports[2], tpids[1]);

    mock_construct_unit_tpid(ports[3].unit, 2, 0x3300);
    mock_construct_port_tpid(ports[3],0x5);
    OUTER_TPID *ot_2_2 = new OUTER_TPID(ports[3], 0x3300);

    mock_construct_unit_tpid(ports[3].unit, 3, 0x4400);
    mock_construct_port_tpid(ports[3],0xd);
    OUTER_TPID *ot_2_3 = new OUTER_TPID(ports[3], 0x4400);

    EXPECT_THROW(OUTER_TPID(ports[2],0x5500), EXP_ERROR);

    mock_construct_port_tpid(ports[0],0x3);
    delete ot_1;

    mock_construct_port_tpid(ports[1],0x3);
    delete ot_1_2;

    mock_construct_port_tpid(ports[1],0x1);
    delete ot_1_3;

    mock_construct_port_tpid(ports[0],0x1);
    delete ot_1_4;

    mock_construct_port_tpid(ports[2],0x1);
    delete ot_2;

    mock_construct_port_tpid(ports[3],0x9);
    delete ot_2_2;

    mock_construct_port_tpid(ports[3],0x1);
    delete ot_2_3;

}

TEST_F(TPID_test, inner_tpid)
{
    INNER_TPID ok_tpid(ports[0], 0x8100);

    EXPECT_THROW(INNER_TPID(ports[0], tpids[1]), EXP_ERROR);
}
