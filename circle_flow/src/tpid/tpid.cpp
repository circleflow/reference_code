
#include "hal_tpid.h"
#include "defs.h"
#include "tpid.h"
#include "init.h"
#include "hal.h"
#include "port_impl.h"

#include <algorithm>
using std::for_each;

#define RESERVED_TPID 0x8100

#include "utility/export/unique.h"
#include "utility/export/smart_ptr_u.h"

#include <map>
using std::multimap;
using std::pair;
using std::make_pair;

/* ============== HW TPID resource ================== */
class UNIT_TPID {
public:
    UNIT_TPID(UINT8 _unit, UINT16 _tpid)
    :unit(_unit),
     tpid(_tpid)
    {
        DB::iterator it=db.find(make_pair(unit,tpid));
        if(it != db.end()) {
            TRC_VERBOSE("unit(%d) tpid(%x) exist, index(%d)\r\n", unit, tpid, it->second->get_id());

            db.insert(make_pair(make_pair(unit,tpid), it->second));
            index = it->second->get_id();
        } else {

            shared_ptr<INDEX> ptr(new INDEX(unit));
            index = ptr->get_id();

            TRC_VERBOSE("unit(%d) tpid(%x) created, index(%d)\r\n", unit, tpid, index);

            hal_tpid_unit_set(unit, index, tpid);
            db.insert(make_pair(make_pair(unit,tpid), ptr));
         }
    }

    ~UNIT_TPID()
    {
        DB::iterator it=db.find(make_pair(unit,tpid));
        ASSERT(it!=db.end());
        db.erase(it);

        TRC_VERBOSE("unit(%d) tpid(%x) removed, index(%d)\r\n", unit, tpid, index);
    }

    UINT8 get_idx() const { return index; }

private:
    UINT8  unit;
    UINT16 tpid;
    UINT8 index;

    //tpid index allocation per unit, unit index as pool index
    typedef UNIQUE<UNIT_TPID, UINT8, 0, 0, unique_id_val<UINT8,0>, hal_tpid_max_id> INDEX;
    typedef multimap<pair<UINT8, UINT16>, shared_ptr<INDEX> > DB;
    static DB db;

};

UNIT_TPID::DB UNIT_TPID::db;

typedef multimap<pair<PORT_ID,UINT16>, shared_ptr<UNIT_TPID> > DB_PORT_TPID;
static DB_PORT_TPID db_port_tpid;

static
void update_port_tpid_bmp(const PORT_ID &port)
{
    UINT8 bmp = 0;

    for(DB_PORT_TPID::iterator it=db_port_tpid.begin(); it!=db_port_tpid.end(); it++){
        if(it->first.first == port) {
            bmp |= (1<<(it->second->get_idx()));
        }
    }

    hal_tpid_port_set(port.unit, port.index, bmp);
}

/* ============== SW TPID class ================== */

OUTER_TPID::OUTER_TPID(const PORT_ID &_port, UINT16 _tpid)
:port(_port),
 tpid(_tpid)
{
    DB_PORT_TPID::iterator it=db_port_tpid.find(make_pair(port,tpid));
    if(it != db_port_tpid.end()) {
        db_port_tpid.insert(make_pair(make_pair(port,tpid), it->second));
    } else {
        shared_ptr<UNIT_TPID> ptr(new UNIT_TPID(port.unit, tpid));
        db_port_tpid.insert(make_pair(make_pair(port,tpid),ptr));
        update_port_tpid_bmp(port);
    }
}

OUTER_TPID::~OUTER_TPID()
{
    DB_PORT_TPID::iterator it=db_port_tpid.find(make_pair(port,tpid));
    ASSERT(it!=db_port_tpid.end());
    db_port_tpid.erase(it);

    if(db_port_tpid.end() == db_port_tpid.find(make_pair(port,tpid))) {
        update_port_tpid_bmp(port);
    }
}

INNER_TPID::INNER_TPID(const PORT_ID &, UINT16 tpid)
{
    ASSERT(tpid==RESERVED_TPID);
}


static
void _port_init(const PORT_NAME_SET::value_type &port_name)
{
    PORT_ID front = PORT_IMPL::get_front(port_name);
    PORT_ID engine = PORT_IMPL::get_engine(port_name);

    //no free
    new OUTER_TPID(front,  RESERVED_TPID);
    new OUTER_TPID(engine, RESERVED_TPID);
}

void CIRCLE_FLOW::tpid_init(void)
{
    //initial setting
    PORT_NAME_SET ports_all = PORT_IMPL::get_port_all();
    for_each(ports_all.begin(), ports_all.end(), _port_init);

    UINT8 unit;
    UNIT_ALL_ITER(unit) {
        hal_tpid_egr_outer_set(unit, RESERVED_TPID);
        hal_tpid_inner_set(unit, RESERVED_TPID);
    }
}







