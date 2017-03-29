
#ifndef OBSERVER_H_
#define OBSERVER_H_

#include "type_ext.h"
#include "tx.h"
#include "cpu.h"

#include <map>
using std::map;
using std::pair;
using std::make_pair;

typedef map<pair<UINT8, UINT16>, CPU::RX::PKT_CB > DB_RX;
static DB_RX db_rx;

static
void add_observer(UINT8 unit, UINT16 vid, const CPU::RX::PKT_CB &cb)
{
    DB_RX::iterator it = db_rx.find(make_pair(unit, vid));

    ASSERT(it == db_rx.end());

    db_rx.insert(make_pair(make_pair(unit,vid), cb));
}

static
void remove_observer(UINT8 unit, UINT16 vid)
{
    DB_RX::iterator it = db_rx.find(make_pair(unit, vid));

    ASSERT(it != db_rx.end());

    db_rx.erase(make_pair(unit,vid));
}

static
void observer_dispatch (UINT8 unit, const PACKET &pkt, UINT32 time_stamp)
{
    DB_RX::iterator it = db_rx.find(make_pair(unit, extract_ovid(pkt)));

    if(it != db_rx.end()) {
        PACKET rm_otag;
        remove_otag(pkt, rm_otag);

        (it->second)(unit, rm_otag, time_stamp);
    }
}

#endif
