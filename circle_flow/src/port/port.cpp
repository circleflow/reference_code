
#include "hal_port.h"
#include "port_impl.h"
#include "port.h"
#include "defs.h"
#include "init.h"

#include <map>
using std::map;
using std::pair;
#include <utility>
using std::make_pair;
#include <algorithm>
using std::sort;
using std::for_each;

namespace CIRCLE_FLOW {
    namespace PORT_IMPL {
        void pair_notify_dispatch(const PORT_NAME &port, const PORT_ID &front, const PORT_ID &engine);
        void link_notify_dispatch(UINT8 unit, UINT8 port, bool link);
    }
}

////////////////port name////////////////////
static PORT_NAME_SET db_port_name_set;

class extract_port_name {
public:
    void operator () (const DEVICE::CONFIG::PAIR_PROFILE &profile) const {
        db_port_name_set.insert(profile.name);
    }
};

PORT_NAME_SET CIRCLE_FLOW::PORT_IMPL::get_port_all(void)
{
    return db_port_name_set;
}

bool CIRCLE_FLOW::PORT_IMPL::is_valid(const PORT_NAME &port)
{
    if(db_port_name_set.find(port) != db_port_name_set.end()) {
        return true;
    } else {
        return false;
    }
}

bool CIRCLE_FLOW::PORT_IMPL::is_valid(const PORT_NAME_SET &ports)
{
    for(PORT_NAME_SET::const_iterator it=ports.begin();
            it!=ports.end(); it++) {
        if(false == is_valid(*it)) {
            return false;
        }
    }

    return true;
}

//////////// front port name //////////

typedef map<PORT_ID, PORT_NAME> DB_PORT_NAME;

static DB_PORT_NAME db_front_port;

PORT_NAME CIRCLE_FLOW::PORT_IMPL::get_port_name(const PORT_ID &pid)
{
    DB_PORT_NAME::iterator it = db_front_port.find(pid);
    if(it != db_front_port.end()){
        return it->second;
    } else {
        TRC_VERBOSE("\r\n unit:%d port:%d not in the name list.", pid.unit, pid.index);
        return PORT_NAME_NULL;
    }
}

////////////////port pair////////////////////

typedef map< pair<PORT_NAME, INTERFACE>, PAIR > DB_PORT_PAIR;

static DB_PORT_PAIR db_port_pair;

PAIR CIRCLE_FLOW::PORT_IMPL::get_pair(const PORT_NAME &port, INTERFACE itf)
{
    DB_PORT_PAIR::iterator it = db_port_pair.find(make_pair(port, itf));
    ASSERT(it != db_port_pair.end());
    return it->second;
}

PAIR CIRCLE_FLOW::PORT_IMPL::get_pair(const PORT_NAME &port)
{
    INTERFACE itf = PORT_IMPL::get_itf(port);

    return get_pair(port, itf);
}

class extract_port_pair {
public:
    void operator () (const DEVICE::CONFIG::PAIR_PROFILE &profile) const {
        for(UINT8 i=0; i<profile.port_pair.size(); i++) {

            const DEVICE::CONFIG::PAIR_PROFILE::PORT_PAIR &port_pair = profile.port_pair[i];

            PORT_ID engine(profile.unit, port_pair.engine);
            PORT_ID front(profile.unit, port_pair.front);
            db_port_pair.insert(make_pair(make_pair(profile.name, port_pair.phy_itf),
                                PAIR(engine, front)));
        }
    }
};

////////////////port itf set////////////////////
typedef map<PORT_NAME, ITF_SET > DB_PORT_ITF_SET;
static DB_PORT_ITF_SET db_port_itf_set;

ITF_SET CIRCLE_FLOW::PORT_IMPL::get_itf_set(const PORT_NAME &port_name)
{
    DB_PORT_ITF_SET::iterator it = db_port_itf_set.find(port_name);
    ASSERT(it != db_port_itf_set.end());

    return it->second;
}

class extract_port_itf_set {
public:
    void operator () (const DEVICE::CONFIG::PAIR_PROFILE &profile) const {

        ITF_SET itfs;

        for(UINT8 i=0; i<profile.port_pair.size(); i++) {

            const DEVICE::CONFIG::PAIR_PROFILE::PORT_PAIR &port_pair = profile.port_pair[i];

            itfs.insert( port_pair.phy_itf);

        }

        db_port_itf_set.insert(make_pair(profile.name, itfs));
    }
};

////////////////port itf specified////////////////////
typedef map<PORT_NAME, INTERFACE > DB_PORT_ITF;

static DB_PORT_ITF db_port_itf;

INTERFACE CIRCLE_FLOW::PORT_IMPL::get_itf(const PORT_NAME &port)
{
    DB_PORT_ITF::iterator it = db_port_itf.find(port);
    ASSERT(it != db_port_itf.end());
    return it->second;
}

class extract_port_itf {
public:
    void operator () (const DEVICE::CONFIG::PAIR_PROFILE &profile) const {

        ASSERT(profile.port_pair.size()>0);

        //take the first definition of pair as the default itf setting
        const DEVICE::CONFIG::PAIR_PROFILE::PORT_PAIR &port_pair = profile.port_pair[0];

        db_port_itf.insert( make_pair(profile.name, port_pair.phy_itf));

        db_front_port.insert(make_pair(PORT_ID(profile.unit,port_pair.front), profile.name));
    }
};

void CIRCLE_FLOW::PORT_IMPL::set_itf(const PORT_NAME &port, INTERFACE itf)
{
    ASSERT(db_port_itf.find(port) != db_port_itf.end());

    if(itf != db_port_itf[port]) {

        INTERFACE old_itf = get_itf(port);
        db_front_port.erase(get_pair(port, old_itf).front);
        db_front_port.insert(make_pair(get_pair(port, itf).front, port));

        db_port_itf[port] = itf;

        //notify
        PAIR pair = get_pair(port, itf);
        pair_notify_dispatch(port, pair.front, pair.engine);
    }
}


ABILITY CIRCLE_FLOW::PORT_IMPL::get_ability(const PORT_ID &port)
{
    return hal_port_ability_get(port.unit, port.index);
}

static
bool is_half_duplex(DUPLEX_SPEED ds)
{

    switch(ds)
    {
        case HD_10MB:
        case HD_100MB:
        case HD_1000MB:
        case HD_2500MB:
        case HD_10GB:
            return true;
        default:
            return false;
    }
}

static
void check_half_duplex(DUPLEX_SPEED ds)
{
    ENSURE(!is_half_duplex(ds), "half duplex not supported");
}

static
void check_half_duplex_s(const DS_SET &ds_set)
{
    for_each(ds_set.begin(), ds_set.end(), check_half_duplex);
}

void CIRCLE_FLOW::PORT_IMPL::set_auto_nego(const PORT_ID &port, const ADVERT &advert)
{
    check_half_duplex_s(advert.ds_set);

    hal_port_auto_nego(port.unit, port.index, advert);
}

void CIRCLE_FLOW::PORT_IMPL::set_forced(const PORT_ID &port, const MODE &mode)
{
    check_half_duplex(mode.ds);

    hal_port_forced(port.unit, port.index, mode);

}

STATUS CIRCLE_FLOW::PORT_IMPL::get_status(const PORT_ID &port)
{
    return hal_port_status_get(port.unit, port.index);
}

PORT_ID CIRCLE_FLOW::PORT_IMPL::get_front(const PORT_NAME &port_name)
{
    return get_pair(port_name).front;
}

PORT_ID CIRCLE_FLOW::PORT_IMPL::get_engine(const PORT_NAME &port_name)
{
    return get_pair(port_name).engine;
}

static
DUPLEX_SPEED max_ds(const DS_SET &ds_set)
{
    ENSURE(ds_set.size()>0, "duplex speed set is empty");

    DUPLEX_SPEED ds = DS_END;

    for(DS_SET::const_iterator it=ds_set.begin() ; it!=ds_set.end(); it++) {
        if(is_half_duplex(*it)) {
            continue;
        } else {
            if(DS_END==ds || *it>ds) {
                ds = *it;
            }
        }
    }

    ENSURE(ds != DS_END, "invalid duplex speed set");

    return ds;
}

static
void latency_port_init(const vector<DEVICE::CONFIG::LATENCY_PROFILE> & profile)
{
    for(UINT8 i=0; i<profile.size(); i++) {

        PORT_ID port(profile[i].unit, profile[i].port);

        ABILITY ability = get_ability(port);

        MODE mode;
        mode.ds = max_ds(ability.ds_set);
        mode.pause = false;
        set_forced(port, mode);

    }
}

static
void pair_port_init(const vector<DEVICE::CONFIG::PAIR_PROFILE> & profile)
{

    //init db
    for_each(profile.begin(),
             profile.end(),
             extract_port_name());

    for_each(profile.begin(),
             profile.end(),
             extract_port_pair());

    for_each(profile.begin(),
             profile.end(),
             extract_port_itf_set());

    //take the first definition of pair as the default itf setting
    for_each(profile.begin(),
             profile.end(),
             extract_port_itf());

    //init port
    for(UINT8 i=0; i<profile.size(); i++) {

        PORT_NAME port_name = profile[i].name;

        PORT_ID front = get_front(port_name);
        ABILITY ability = get_ability(front);
        if(ability.auto_nego) {
            ADVERT advert;
            advert.ds_set = ability.ds_set;
            advert.pause = ability.pause;
            set_auto_nego(front, advert);
        } else {
            MODE mode;
            mode.ds = max_ds(ability.ds_set);
            mode.pause = ability.pause;
            set_forced(front, mode);
        }
    }
}

static
void on_pair_change(const PORT_NAME &port, const PORT_ID &front, const PORT_ID &engine)
{
    hal_port_cnt_clear(front.unit,  front.index);
    hal_port_cnt_clear(engine.unit, engine.index);
}

void CIRCLE_FLOW::port_init(const vector<DEVICE::CONFIG::PAIR_PROFILE> & pair_profile,
                            const vector<DEVICE::CONFIG::LATENCY_PROFILE> & latency_profile,
                            UINT32 max_frame_size)
{
    pair_port_init(pair_profile);

    latency_port_init(latency_profile);

    hal_port_link_callback_register(link_notify_dispatch);

    //start rate cac timer
    PORT_IMPL::get_rate(*(PORT_IMPL::get_port_all().begin()));

    //clear hw cnt once pair changed
    pair_notify_bind(persistent_shared<PAIR_CB>(on_pair_change));

    hal_port_max_frame_size_set(max_frame_size);
}

static
bool is_valid(INTERFACE itf)
{
    return itf < ITF_END ;
}

//////////////// external API //////////////////
ITF_SET CIRCLE_FLOW::PORT::get_itf_set(const PORT_NAME &port)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ENSURE_INIT_OK;

    AUTO_MUTEX_API;

    ENSURE(PORT_IMPL::is_valid(port),
            "invalid port \"%s\"", port.c_str());

    return PORT_IMPL::get_itf_set(port);

    EXP_RECAP_END;
}

void CIRCLE_FLOW::PORT::set_itf(const PORT_NAME &port, INTERFACE itf)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ENSURE_INIT_OK;

    AUTO_MUTEX_API;

    ENSURE(PORT_IMPL::is_valid(port),
            "invalid port \"%s\"", port.c_str());

    ENSURE(is_valid(itf), "invalid interface");

    PORT_IMPL::set_itf(port, itf);

    EXP_RECAP_END;
}

INTERFACE CIRCLE_FLOW::PORT::get_itf(const PORT_NAME &port)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ENSURE_INIT_OK;

    AUTO_MUTEX_API;

    ENSURE(PORT_IMPL::is_valid(port),
            "invalid port \"%s\"", port.c_str());

    return PORT_IMPL::get_itf(port);

    EXP_RECAP_END;
}

ABILITY CIRCLE_FLOW::PORT::get_ability(const PORT_NAME &port)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ENSURE_INIT_OK;

    AUTO_MUTEX_API;

    ENSURE(PORT_IMPL::is_valid(port),
            "invalid port \"%s\"", port.c_str());

    return PORT_IMPL::get_ability(PORT_IMPL::get_front(port));

    EXP_RECAP_END;
}

void CIRCLE_FLOW::PORT::set_auto_nego(const PORT_NAME &port, const ADVERT &advert)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ENSURE_INIT_OK;

    AUTO_MUTEX_API;

    ENSURE(PORT_IMPL::is_valid(port),
            "invalid port \"%s\"", port.c_str());

    check_half_duplex_s(advert.ds_set);

    PORT_IMPL::set_auto_nego(PORT_IMPL::get_front(port), advert);

    EXP_RECAP_END;
}


void CIRCLE_FLOW::PORT::set_forced(const PORT_NAME &port, const PORT::MODE &mode)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ENSURE_INIT_OK;

    AUTO_MUTEX_API;

    ENSURE(PORT_IMPL::is_valid(port),
            "invalid port \"%s\"", port.c_str());

    check_half_duplex(mode.ds);

    PORT_IMPL::set_forced(PORT_IMPL::get_front(port), mode);

    EXP_RECAP_END;
}

STATUS CIRCLE_FLOW::PORT::get_status(const PORT_NAME &port)
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ENSURE_INIT_OK;

    AUTO_MUTEX_API;

    ENSURE(PORT_IMPL::is_valid(port),
            "invalid port \"%s\"", port.c_str());

    return PORT_IMPL::get_status(PORT_IMPL::get_front(port));

    EXP_RECAP_END;
}

PORT_NAME_SET CIRCLE_FLOW::PORT::get_port_all()
{
    EXP_RECAP_START;

    AUTO_TRC_FUNC;

    ENSURE_INIT_OK;

    AUTO_MUTEX_API;

    return PORT_IMPL::get_port_all();

    EXP_RECAP_END;
}

//////////////// helper ///////////////

PORT_NAME_SET CIRCLE_FLOW::PORT_IMPL::make_ports(const PORT_NAME_SET &_ports, const PORT_NAME &port)
{
    PORT_NAME_SET ports = _ports;
    if(ports.end() == find(ports.begin(), ports.end(), port)) {
        ports.insert(port);
    }

    return ports;
}

PORT_NAME_SET CIRCLE_FLOW::PORT_IMPL::make_ports(const PORT_NAME &port)
{
    PORT_NAME_SET ports;
    ports.insert(port);
    return ports;
}

bool CIRCLE_FLOW::PORT_IMPL::is_same_unit(const PORT_NAME_SET &ports)
{
    if(ports.size()<=1) {
        return true;
    }

    PORT_NAME_SET::const_iterator it = ports.begin();
    PORT_ID port = PORT_IMPL::get_front(*it);

    UINT8 unit = port.unit;

    for(it++; it!=ports.end(); it++) {
        port = PORT_IMPL::get_front(*it);
        if(unit != port.unit) {
            return false;
        }
    }

    return true;
}

bool CIRCLE_FLOW::PORT_IMPL::port_link(const PORT_ID &port)
{
    STATUS status = PORT_IMPL::get_status(port);
    return status.link;
}

