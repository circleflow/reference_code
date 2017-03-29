/*
 * mmu_test.cpp
 *
 *  Created on: 2014-9-10
 *      Author: jingpa
 */

#include "test_helper.h"

#include "hal.h"
#include "mmu.h"
#include "type_ext.h"
#include "hal_mmu_stub.hpp"
#include "init.h"

#include "utility/export/error.h"
#include "utility/export/smart_ptr_u.h"

#include "test_helper.h"



class MMU_test : public ::testing::Test {
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

static
const HAL_MMU_SPECS & mmu_info_cb(UINT8 unit, int calls)
{
    (void) unit;
    (void) calls;

    static HAL_MMU_SPECS specs;

#define BYTE_PER_CELL 128
    specs.byte_per_cell = BYTE_PER_CELL;
    specs.total_cell    = 8 * 1024;
    specs.total_pkt     = 1 * 1024;

    return specs;
}

static UINT32 free_cell[2];
static void mmu_share_set_cb(UINT8 unit, UINT32 ing_max_cell, UINT32 ing_limit_pkt,
                             UINT32 egr_max_cell, UINT32 egr_max_pkt, int calls)
{
    (void) ing_max_cell;
    (void) ing_limit_pkt;
    (void) egr_max_pkt;

    if(1 == calls) {
        EXPECT_EQ(0, unit);
        free_cell[0] = egr_max_cell;
    } else if( 2==calls ) {
        EXPECT_EQ(1, unit);
        free_cell[1] = egr_max_cell;
    } else {
        ADD_FAILURE() << "unexpected call of mmu_share_set, calls: "<<calls;
    }
}

const HAL_UNIT_SPECS & CIRCLE_FLOW::hal_unit_specs(UINT8 unit)
{
    (void) unit;

    static HAL_UNIT_SPECS specs;

    specs.cpu_port_id = 0;

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

#include "queue.h"
UINT8 QUEUE::num_of_q(UINT8 unit, UINT8 port)
{
    (void) unit;
    (void) port;

    return 4;
}

static vector< shared_ptr<MMU> > v_mmu;
void mmu_alloc(UINT8 unit, UINT32 num_of_cell)
{
    vector<UINT16> pkts_len;

    UINT16 i=1;
    int left_cell = num_of_cell-i;
    for(; left_cell>=0 ;) {
        pkts_len.push_back(i*BYTE_PER_CELL-1);

        i++;

        if((left_cell-i) >= 0) {
            left_cell -= i;
        }
        else {
            if(left_cell>0) {
                pkts_len.push_back(left_cell*BYTE_PER_CELL-1);
            }

            break;
        }
    }

    MMU *mmu = new MMU(unit, pkts_len);
    v_mmu.push_back(shared_ptr<MMU> (mmu));
}

void mmu_free_all(void)
{
    v_mmu.clear();
}


TEST_F(MMU_test, basic)
{
    hal_mmu_specs_MockWithCallback(mmu_info_cb);

    hal_mmu_share_set_MockWithCallback(mmu_share_set_cb);

    free_cell[0] = 0;
    free_cell[1] = 0;

    CIRCLE_FLOW::mmu_init();

    EXPECT_TRUE(free_cell[0] > 0);
    EXPECT_TRUE(free_cell[1] > 0);

    for(UINT8 repeat=0; repeat<3; repeat++) {

        for(UINT8 unit=0; unit<2; unit++) {

            UINT32 i=1;
            int lef_cell = free_cell[unit] - i;
            for(; lef_cell>0; ) {
                mmu_alloc(unit, i);
                i++;
                lef_cell -= i;
            }

            EXPECT_THROW(mmu_alloc(unit, i), EXP_ERROR);
        }

        mmu_free_all();
    }
}
