
#include "defs.h"
#include "device.h"
#include "cfg_check.h"
#include "init.h"
using namespace CIRCLE_FLOW;

#include "utility/export/error.h"
#include "utility/export/trace.h"
#include "utility/export/mutex_u.h"
using namespace UTILITY;

static bool is_init_called = false;
static bool is_init_ok = false;

void DEVICE::init(const CONFIG & config)
{

    EXP_RECAP_START;

    ENSURE(false == is_init_called);
    is_init_called = true;

    AUTO_MUTEX_API;

    cfg_check(config);

    fp_init();

    queue_init();

    cpu_init();

    port_init(config.pair_profile,
              config.latency_profile,
              config.max_frame_size);

    mmu_init();

    tx_init(config.latency_profile);

    tpid_init();

    is_init_ok = true;

    EXP_RECAP_END;
}

bool DEVICE::is_init_done()
{
    return is_init_ok;
}
