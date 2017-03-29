#include "utility/export/error.h"
#include "utility/export/base_type.h"
#include "utility/export/trace.h"
using namespace UTILITY;

#include "circle_flow/export/device.h"
#include "circle_flow/export/port.h"
using namespace CIRCLE_FLOW;

/* this file contains the device related configuration
 * which need to be adjusted to fit your target device */

struct PORT_PROFILE_GE {
    string name;
    UINT8  unit;
    DEVICE::CONFIG::PAIR_PROFILE::PORT_PAIR pair_0, pair_1;
};

static
DEVICE::CONFIG::PAIR_PROFILE cvt (const PORT_PROFILE_GE &ref)
{
    DEVICE::CONFIG::PAIR_PROFILE profile;
    profile.name = ref.name;
    profile.unit = ref.unit;
    profile.port_pair.push_back(ref.pair_0);
    profile.port_pair.push_back(ref.pair_1);

    return profile;
}

#define RJ45 PORT::ITF_RJ45
#define SFP  PORT::ITF_SFP

static  PORT_PROFILE_GE profile[] = {
        // name  unit     pair-0      pair-1
        {"ge00",  0,  {SFP, 3, 2}, {RJ45, 2, 3}},
        {"ge01",  0,  {SFP, 5, 4}, {RJ45, 4, 5}},
        {"ge02",  0,  {SFP, 7, 6}, {RJ45, 6, 7}},
        {"ge03",  0,  {SFP, 9, 8}, {RJ45, 8, 9}},
        {"ge04",  0,  {SFP,11,10}, {RJ45,10,11}},
        {"ge05",  0,  {SFP,13,12}, {RJ45,12,13}},
        {"ge06",  0,  {SFP,15,14}, {RJ45,14,15}},
        {"ge07",  0,  {SFP,17,16}, {RJ45,16,17}},
        {"ge08",  0,  {SFP,19,18}, {RJ45,18,19}},
        {"ge09",  1,  {SFP, 3, 2}, {RJ45, 2, 3}},
        {"ge10",  1,  {SFP, 5, 4}, {RJ45, 4, 5}},
        {"ge11",  1,  {SFP, 7, 6}, {RJ45, 6, 7}},
        {"ge12",  1,  {SFP, 9, 8}, {RJ45, 8, 9}},
        {"ge13",  1,  {SFP,11,10}, {RJ45,10,11}},
        {"ge14",  1,  {SFP,13,12}, {RJ45,12,13}},
        {"ge15",  1,  {SFP,15,14}, {RJ45,14,15}},
        {"ge16",  1,  {SFP,17,16}, {RJ45,16,17}},
        {"ge17",  1,  {SFP,19,18}, {RJ45,18,19}}
    };


static
void port_init(void)
{
    PORT_NAME_SET ports = PORT::get_port_all();

    PORT::MODE mode;
    mode.ds = PORT::FD_100MB;
    mode.pause = false;

    for(PORT_NAME_SET::iterator it = ports.begin(); it!=ports.end(); it++) {
        PORT::set_forced(*it, mode);
    }
}

#define SIZE_OF_PROFILE (sizeof(profile)/sizeof(PORT_PROFILE_GE))

namespace SYS_ADAPT {
    void device_init(void)
    {
        DEVICE::CONFIG cfg;

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

        cfg.max_frame_size = 8192;

        DEVICE::init(cfg);

        port_init();
    }
}

