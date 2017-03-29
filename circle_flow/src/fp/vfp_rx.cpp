
#include "vfp_entry.h"
#include "vfp_rx.h"
#include "type_ext.h"
#include "qual.h"
#include "utility/export/smart_ptr_u.h"
#include "utility/export/error.h"
#include "utility/export/unique.h"
#include <map>
using std::multimap;
using std::pair;
using std::make_pair;

/* VFP has no port bitmap for qualify
 * if the qualify quals on mulptile ports, we need multiple entry each for one port
 * thus, the CLASS_ID should be same one for these entry.
 * since VFP only quals part of the RX qual fields,
 * it is possible that different RX qual has the same of VFP part,
 * thus, the CLASS_ID could be shared for different RX qual, in order to save CLASS_ID resource.
 */

#if defined(CF_BCM_56334)
#define MAX_VFP_CLASS_ID 1023
#else
"chip type not suppoted yet, pls complete it here."
#endif

enum ENUM_CLASS_ID {};
typedef UNIQUE<ENUM_CLASS_ID, UINT16, 1, MAX_VFP_CLASS_ID> UNIQUE_CLASS_ID;

struct KEY_CLASS_ID{
    const UINT8 unit;
    const QUAL::PDF_RULES rules;

    KEY_CLASS_ID(UINT8 _unit, const QUAL::PDF_RULES &_rules)
    :unit(_unit),rules(_rules) {}

    bool operator < (const KEY_CLASS_ID &ref) const
    { return make_pair(unit, rules) < make_pair(ref.unit, ref.rules); }
};

/* sharing CLASS_ID among different RX quals */
typedef multimap< KEY_CLASS_ID, shared_ptr<UNIQUE_CLASS_ID> > MAP_CLASS_ID;
static  MAP_CLASS_ID db_class_id;

struct KEY_VFP_ENTRY{
    const UINT8 unit;
    const UINT8 port;
    const VFP_CLASS_ID class_id;

    KEY_VFP_ENTRY(UINT8 _unit, UINT8 _port, VFP_CLASS_ID _class_id)
    :unit(_unit),port(_port),class_id(_class_id) {}

    bool operator < (const KEY_VFP_ENTRY &ref) const
    { return make_pair(make_pair(unit,port),class_id)
            < make_pair(make_pair(ref.unit,ref.port),ref.class_id); }
};

/* sharing VFP_ENTRY among different VFP quals */
typedef multimap< KEY_VFP_ENTRY, shared_ptr<VFP_ENTRY> > MAP_VFP_ENTRY;
static MAP_VFP_ENTRY db_vfp_entry;

VFP_RX::VFP_RX(UINT8 _unit, const vector<UINT8> &_ports, const QUAL::PDF_RULES &_rules, UINT8 pri)
:unit(_unit),
 ports(_ports),
 rules(_rules)
{
    ASSERT(ports.size()>0);

    if(0 == rules.size()) {
        class_id = NULL_VFP_CLASS_ID;
        return;
    }

    {
        KEY_CLASS_ID key(unit,rules);
        MAP_CLASS_ID::iterator it = db_class_id.find(key);
        if( it!=db_class_id.end() ) {
            db_class_id.insert(make_pair(key, it->second));
            class_id = it->second->get_id();
        } else {
            shared_ptr<UNIQUE_CLASS_ID> ptr(new UNIQUE_CLASS_ID(unit));
            db_class_id.insert(make_pair(key, ptr));
            class_id = ptr->get_id();
        }
    }

    for(vector<UINT8>::const_iterator it_port=ports.begin(); it_port!=ports.end(); it_port++){
        KEY_VFP_ENTRY key(unit,*it_port,class_id);

        MAP_VFP_ENTRY::iterator it_db = db_vfp_entry.find(key);
        if( it_db!=db_vfp_entry.end() ) {
            db_vfp_entry.insert(make_pair(key, it_db->second));
        } else {
            shared_ptr<VFP_ENTRY> ptr(new VFP_ENTRY(unit,*it_port,class_id,rules, pri));
            db_vfp_entry.insert(make_pair(key, ptr));
        }
    }
}


VFP_RX::~VFP_RX()
{
    if(NULL_VFP_CLASS_ID == class_id) {
        return;
    }

    for(vector<UINT8>::const_iterator it_port=ports.begin(); it_port!=ports.end(); it_port++){
        MAP_VFP_ENTRY::iterator it_db = db_vfp_entry.find(KEY_VFP_ENTRY(unit,*it_port,class_id));
        ASSERT(it_db != db_vfp_entry.end());
        db_vfp_entry.erase(it_db);
    }

    {
        MAP_CLASS_ID::iterator it = db_class_id.find(KEY_CLASS_ID(unit,rules));
        ASSERT(it != db_class_id.end());
        db_class_id.erase(it);
    }
}



