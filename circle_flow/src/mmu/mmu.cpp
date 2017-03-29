

#include "hal_mmu.h"
#include "port_impl.h"
#include "mmu.h"
#include "hal.h"
#include "queue.h"
#include "init.h"

#include "utility/export/error.h"

#include <map>
using std::map;

#include <utility>
using std::make_pair;


/* MMU designed to check if there's enough cell available for pkts pending in engine port queue.
 * during initialization, part of cells allocated to guarantee front port and CPU port,
 * the left cells then are used for engine ports.
 * engine port queue threshold are fixed to total cells number,
 * so the availability checking is need only performed in software level.
 * in case of port pair change, port queue threshold need to be update depend on front/engine.
 * */

static HAL_MMU_SPECS specs;
#define TOTAL_CELL    specs.total_cell
#define TOTAL_PKT     specs.total_pkt
#define BYTE_PER_CELL specs.byte_per_cell

// garantee the minimal requirement for standard frame and jumbo frame
#define ETH_STANDARD_FRAME 1536
#define ETH_JUMBO_FRAME    9216

#define MAX_PORT_CELL (ETH_JUMBO_FRAME/BYTE_PER_CELL)
#define MIN_Q_CELL    (ETH_STANDARD_FRAME/BYTE_PER_CELL)

// CPU queue need more buffer, because more pkts would be pending in queue than ethernet port
#define MMU_CPU_MAX_BYTES   (128*1024)
#define MMU_CPU_MIN_BYTES    (16*1024)

#define MAX_CPU_PORT_CELL  (MMU_CPU_MAX_BYTES/BYTE_PER_CELL)
#define MIN_CPU_Q_CELL     (MMU_CPU_MIN_BYTES/BYTE_PER_CELL)

typedef map<UINT8, UINT32> DB_FREE_CELL;
static DB_FREE_CELL db_free_cell;

static
void _mmu_init(UINT8 unit)
{
    UINT8 port;
    UINT32 free_cell = TOTAL_CELL;

    // ingress based thresholds, disabled back pressure
    PORT_E_ALL_ITER(unit, port) {
        hal_mmu_ingress_set(unit, port, TOTAL_CELL, TOTAL_PKT);
    }

    // egress based thresholds
    {
        //cpu port
        UINT8 cpu_port_id = hal_unit_specs(unit).cpu_port_id;
        int cpu_q_num = QUEUE::num_of_q(unit, cpu_port_id);

        hal_mmu_egress_set(unit, cpu_port_id, MAX_CPU_PORT_CELL, TOTAL_PKT);
        free_cell -= (MAX_CPU_PORT_CELL-MIN_CPU_Q_CELL);

        for(int i=0; i<cpu_q_num; i++) {
            hal_mmu_egress_q_set(unit, cpu_port_id, i,
                             MIN_CPU_Q_CELL, TOTAL_PKT,
                             MAX_CPU_PORT_CELL, TOTAL_PKT);
            free_cell -= MIN_CPU_Q_CELL;
        }
    }

    {
        // ethernet ports
        // once port pair mode determined(engine or front),
        // engine port will be re-configured with full capacity of max cell

        PORT_E_ALL_ITER(unit, port) {

            hal_mmu_egress_set(unit, port, MAX_PORT_CELL, TOTAL_PKT);
            free_cell -= (MAX_PORT_CELL-MIN_Q_CELL);

            int q_num = QUEUE::num_of_q(unit, port);

            for(int i=0; i<q_num; i++) {
                hal_mmu_egress_q_set(unit, port, i,
                                 MIN_Q_CELL, TOTAL_PKT,
                                 MAX_PORT_CELL, TOTAL_PKT);
                free_cell -= MIN_Q_CELL;
            }
        }
    }

    db_free_cell.insert(make_pair(unit,free_cell));

    hal_mmu_share_set(unit, TOTAL_CELL, TOTAL_PKT,
                  free_cell, TOTAL_PKT);
}

static
void port_init(UINT8 unit, UINT8 front, UINT8 engine)
{
    // front port only has the minimal garantee
    hal_mmu_egress_set(unit, front, MAX_PORT_CELL, TOTAL_PKT);

    int q_num = QUEUE::num_of_q(unit, front);

    for(int i=0; i<q_num; i++) {
        hal_mmu_egress_q_set(unit, front, i,
                         MIN_Q_CELL, TOTAL_PKT,
                         MAX_PORT_CELL, TOTAL_PKT);
    }

    // engine port has the maximum garantee
    hal_mmu_egress_set(unit, engine, TOTAL_CELL, TOTAL_PKT);

    q_num = QUEUE::num_of_q(unit, engine);

    for(int i=0; i<q_num; i++) {
        hal_mmu_egress_q_set(unit, engine, i,
                         MIN_Q_CELL, TOTAL_PKT,
                         TOTAL_CELL, TOTAL_PKT);
    }
}

static
void on_pair_change (const PORT_NAME &, const PORT_ID &front, const PORT_ID &engine)
{
    port_init(front.unit, front.index, engine.index);
}

void CIRCLE_FLOW::mmu_init(void)
{
    specs = hal_mmu_specs();

    int unit;
    UNIT_ALL_ITER(unit){
        _mmu_init(unit);
    }

    pair_notify_bind(persistent_shared<PAIR_CB>(on_pair_change));
}

MMU::MMU(UINT8 _unit, const vector<UINT16> &pkts_len)
{
    unit = _unit;
    cells = 0;
    for(UINT8 i=0; i<pkts_len.size(); i++) {
        cells += (pkts_len[i]+BYTE_PER_CELL-1)/BYTE_PER_CELL;
    }

    DB_FREE_CELL::iterator it = db_free_cell.find(unit);
    ASSERT(it != db_free_cell.end());

    if(it->second >= cells) {
        it->second -= cells;
    } else {
        ERROR("not enough free cells");
    }
}

MMU::~MMU()
{
    db_free_cell[unit] += cells;
}
