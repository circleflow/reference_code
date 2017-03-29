#include "port_impl.h"
#include "defs.h"

#include <map>
using std::map;
using std::pair;
#include <list>
using std::list;
#include <algorithm>
using std::includes;


//////////////////// link notify ///////////////////////
typedef list< weak_ptr<LINK_CB> > LIST_LINK_NOTIFY;
typedef map<PORT_NAME, LIST_LINK_NOTIFY > DB_LINK_NOTIFY;

static DB_LINK_NOTIFY  db_link_notify;

static bool is_link_cb_expired(const weak_ptr<LINK_CB> &cb)
{
    return cb.expired();
}

void CIRCLE_FLOW::PORT_IMPL::link_notify_bind(const weak_ptr<LINK_CB> &cb,
                                              const PORT_NAME_SET &ports,
                                              bool notify_now)
{

    PORT_NAME_SET ports_all = get_port_all();
    ENSURE(includes(ports_all.begin(), ports_all.end(),
                    ports.begin(), ports.end()));

    for(PORT_NAME_SET::const_iterator it=ports.begin(); it!=ports.end(); it++){
        db_link_notify[*it].push_back(cb);
    }

    //purge invalid cb
    for(DB_LINK_NOTIFY::iterator it=db_link_notify.begin(); it!=db_link_notify.end(); it++) {
        it->second.remove_if(is_link_cb_expired);
    }

    //trigger a notify
    if(notify_now) {
        weak_ptr<LINK_CB> wp(cb);
        for(PORT_NAME_SET::const_iterator it=ports.begin(); it!=ports.end(); it++){
            if(!wp.expired()) {
                (*wp.lock())(*it, port_link(get_front(*it)));
            }
        }
    }
}


namespace CIRCLE_FLOW {
    namespace PORT_IMPL {

void link_notify_dispatch(UINT8 unit, UINT8 port, bool link)
{
    /*under the sdk thread context:
      > exception should be catch locally
      > data race need to be avoided with other thread
    */

    EXP_FREE_START;

    AUTO_TRC_FUNC;

    AUTO_MUTEX_API;

    TRC_VERBOSE("\r\n unit:%d port:%d link changed to %d", unit, port, link);

    PORT_NAME port_name = get_port_name(PORT_ID(unit, port));

    if(port_name != PORT_NAME_NULL) {
        LIST_LINK_NOTIFY &list = db_link_notify[port_name];

        for (LIST_LINK_NOTIFY::iterator it=list.begin(); it!=list.end(); it++) {
            if(!(it->expired())) {
                (*it->lock())(port_name, link);
            }
        }
    }

    EXP_FREE_END_NR;
}

    }
}

//////////////////////////// pair notify /////////////////////////
typedef list< weak_ptr<PAIR_CB> > LIST_PAIR_NOTIFY;
typedef map<PORT_NAME, LIST_PAIR_NOTIFY > DB_PAIR_NOTIFY;

static DB_PAIR_NOTIFY db_pair_notify;

static bool is_pair_cb_expired(const weak_ptr<PAIR_CB> &cb)
{
    return cb.expired();
}

void CIRCLE_FLOW::PORT_IMPL::pair_notify_bind(const weak_ptr<PAIR_CB> &cb, const PORT_NAME_SET &ports, bool notify_now)
{
    PORT_NAME_SET ports_all = get_port_all();
    ENSURE(includes(ports_all.begin(), ports_all.end(),
                ports.begin(), ports.end()));

    for(PORT_NAME_SET::const_iterator it=ports.begin(); it!=ports.end(); it++){
        db_pair_notify[*it].push_back(cb);
    }

    //purge invalid cb
    for(DB_PAIR_NOTIFY::iterator it=db_pair_notify.begin(); it!=db_pair_notify.end(); it++) {
        it->second.remove_if(is_pair_cb_expired);
    }

    //trigger a notify
    if(notify_now) {
        weak_ptr<PAIR_CB> wp(cb);
        for(PORT_NAME_SET::const_iterator it=ports.begin(); it!=ports.end(); it++){
            if(!wp.expired()) {
                PAIR pair = get_pair(*it);
                (*wp.lock())(*it, pair.front, pair.engine);
            }
        }
    }
}

namespace CIRCLE_FLOW {
    namespace PORT_IMPL {

void pair_notify_dispatch(const PORT_NAME &port, const PORT_ID &front, const PORT_ID &engine)
{
    LIST_PAIR_NOTIFY &list = db_pair_notify[port];

    /* note: call back in the reversed order of binding.
     * there's a notify created from port_init(), which is binded prior than any other ones from flow,
     * and it need to be notified after flow ones.
     * as a simple solution (without introduce call back priroity):
     * call them in a reversed order.
     * */

    for(LIST_PAIR_NOTIFY::reverse_iterator it=list.rbegin(); it!=list.rend(); it++) {
        if(!it->expired()) {
            (*it->lock())(port, front, engine);
        }
    }

}

    }
}
