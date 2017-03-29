
#include "gtest/gtest.h"
#include "opmock.h"

#include "device/cfg_check.h"

struct PAIR_PROFILE_GE {
    string name;
    UINT8  unit;
    DEVICE::CONFIG::PAIR_PROFILE::PORT_PAIR port_pair[2];
};

static DEVICE::CONFIG::PAIR_PROFILE cvt (const PAIR_PROFILE_GE &ref)
{
    DEVICE::CONFIG::PAIR_PROFILE profile;
    profile.name = ref.name;
    profile.unit = ref.unit;
    profile.port_pair.push_back(ref.port_pair[0]);
    profile.port_pair.push_back(ref.port_pair[1]);

    return profile;
}


#define RJ45 PORT::ITF_RJ45
#define SFP  PORT::ITF_SFP

static  PAIR_PROFILE_GE profile[] = {
        // name    unit    pair-0         pair-1
        {"ge 0/0",  0, {{RJ45, 2, 3}, {SFP, 3, 2}}},
        {"ge 0/1",  0, {{RJ45, 4, 5}, {SFP, 5, 4}}},
        {"ge 0/2",  0, {{RJ45, 6, 7}, {SFP, 7, 6}}},
        {"ge 0/3",  0, {{RJ45, 8, 9}, {SFP, 9, 8}}},
        {"ge 0/4",  0, {{RJ45,10,11}, {SFP,11,10}}},
        {"ge 0/5",  0, {{RJ45,12,13}, {SFP,13,12}}},
        {"ge 0/6",  0, {{RJ45,14,15}, {SFP,15,14}}},
        {"ge 0/7",  0, {{RJ45,16,17}, {SFP,17,16}}},
        {"ge 0/8",  0, {{RJ45,18,19}, {SFP,19,18}}},
        {"ge 1/0",  1, {{RJ45, 2, 3}, {SFP, 3, 2}}},
        {"ge 1/1",  1, {{RJ45, 4, 5}, {SFP, 5, 4}}},
        {"ge 1/2",  1, {{RJ45, 6, 7}, {SFP, 7, 6}}},
        {"ge 1/3",  1, {{RJ45, 8, 9}, {SFP, 9, 8}}},
        {"ge 1/4",  1, {{RJ45,10,11}, {SFP,11,10}}},
        {"ge 1/5",  1, {{RJ45,12,13}, {SFP,13,12}}},
        {"ge 1/6",  1, {{RJ45,14,15}, {SFP,15,14}}},
        {"ge 1/7",  1, {{RJ45,16,17}, {SFP,17,16}}},
        {"ge 1/8",  1, {{RJ45,18,19}, {SFP,19,18}}}
    };

#define SIZE_OF_PROFILE (sizeof(profile)/sizeof(PAIR_PROFILE_GE))

class DEVICE_test : public ::testing::Test {
 protected:

    DEVICE::CONFIG cfg;

    virtual void SetUp()
    {
        opmock_test_reset();

        for(UINT8 i=0; i<SIZE_OF_PROFILE; i++) {
            DEVICE::CONFIG::PAIR_PROFILE _p = cvt(profile[i]);
            cfg.pair_profile.push_back(_p);
        }

        {
            DEVICE::CONFIG::LATENCY_PROFILE profile;
            profile.unit = 0;
            profile.port = 25;
            cfg.latency_profile.push_back(profile);
            profile.unit = 1;
            cfg.latency_profile.push_back(profile);
        }
    }
};

//manul stub
namespace CIRCLE_FLOW {
    const HAL_UNIT_SPECS & hal_unit_specs(UINT8 unit)
    {
        (void) unit;

        static HAL_UNIT_SPECS specs;

        specs.device_id = 0x33333333;

        return specs;
    }
}

TEST_F(DEVICE_test, cfg_check)
{

    cfg_check(cfg);

    //for error case
    {
        DEVICE::CONFIG _cfg = cfg;

        // repeated unit
        _cfg.latency_profile[1].unit = 0;
        EXPECT_THROW(cfg_check(_cfg), EXP_ERROR);
    }

}

