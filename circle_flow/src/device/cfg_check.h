
#include "device.h"
#include "type_ext.h"
#include "hal.h"

#include "utility/export/error.h"

#include <set>
using std::set;
#include <algorithm>
using std::for_each;
using std::set_intersection;
#include <iterator>
using std::inserter;

struct EXTRACT_DEV_ID {
    set<UINT32> dev_id;

    void operator () (UINT8 unit)
    {
        dev_id.insert(hal_unit_specs(unit).device_id);
    }
};

struct PAIR_CHECK {
    set<UINT8> units;
    set<PORT_NAME> names;
    set<PORT_ID> ports;
    void operator () (const DEVICE::CONFIG::PAIR_PROFILE &profile)
    {
        units.insert(profile.unit);
        names.insert(profile.name);

        ENSURE(profile.port_pair.size()>0 && profile.port_pair.size()<=PORT::ITF_END);

        set<PORT::INTERFACE> itfs;
        set<PORT_ID> pair_ports;
        typedef vector<DEVICE::CONFIG::PAIR_PROFILE::PORT_PAIR>::const_iterator pair_citer;
        for(pair_citer it=profile.port_pair.begin(); it!=profile.port_pair.end(); it++) {
            itfs.insert(it->phy_itf);
            pair_ports.insert(PORT_ID(profile.unit, it->front));
            pair_ports.insert(PORT_ID(profile.unit, it->engine));
        }

        //no duplicated itf
        ENSURE(itfs.size() == profile.port_pair.size());

        set<PORT_ID> result;
        set_intersection(ports.begin(), ports.end(),
                         pair_ports.begin(), pair_ports.end(),
                         inserter(result, result.begin()));

        //no shared port between pairs
        ENSURE(0 == result.size());

        ports.insert(pair_ports.begin(), pair_ports.end());
    }
};

struct LATENCY_CHECK {
    set<UINT8> units;
    void operator () (const DEVICE::CONFIG::LATENCY_PROFILE &profile)
    {

        ENSURE(units.find(profile.unit) == units.end());

        units.insert(profile.unit);
    }
};

static
void cfg_check(const DEVICE::CONFIG &cfg)
{
    ENSURE(cfg.pair_profile.size()>8);

    //port pair check
    PAIR_CHECK pair_check(
            for_each(cfg.pair_profile.begin(), cfg.pair_profile.end(), PAIR_CHECK()));

    //latency check
    LATENCY_CHECK latency_check(
            for_each(cfg.latency_profile.begin(), cfg.latency_profile.end(), LATENCY_CHECK()));

    ENSURE(pair_check.units == latency_check.units);

    //check consistancy of unit type
    set<UINT8> &units = pair_check.units;
    if(units.size()>1) {
        EXTRACT_DEV_ID extract_dev(
            for_each(units.begin(), units.end(), EXTRACT_DEV_ID()));

        ENSURE(1 == extract_dev.dev_id.size());
    }

}


