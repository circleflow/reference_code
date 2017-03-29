
#include "log.h"
using namespace UTILITY;

#include <vector>
using std::vector;

struct LOG_INFO {
    unsigned int id;
    string info;

    LOG_INFO():id(0), info("empty log") {}
    LOG_INFO(int _id, const string &_info) : id(_id), info(_info) {}
};

typedef vector<LOG_INFO> DB_LOG;

#define DB_SIZE (1024*8)
#define ID_TO_IDX(id) ((id)%DB_SIZE)

/* log record and log id are both recycled,
 * the range of log id is defined much larger than num of records,
 * this enable the consistency check on log id and record, at an extent
 *   i.e even records overflow and recycled, but log id not, so won't get a unmatched record by the log id.
 * */
static unsigned short log_id = 0;
#define MAX_LOG_ID 0xffff

static
DB_LOG & get_db()
{
    static vector<LOG_INFO> db_log;
    static bool initilized = false;

    if(false == initilized) {
        db_log.resize(DB_SIZE);
        initilized = true;
    }

    return db_log;
}

unsigned short LOG::log (const string &info)
{
    LOG_INFO log_info(log_id, info);
    DB_LOG &db_log = get_db();

    db_log[ID_TO_IDX(log_id)]=log_info;

    log_id = (log_id+1)%MAX_LOG_ID;

    return log_id;
}

string LOG::get_log(unsigned short log_id)
{
    DB_LOG &db_log = get_db();

    if(db_log[ID_TO_IDX(log_id)].id == log_id) {
        return db_log[ID_TO_IDX(log_id)].info;
    } else {
        return string("outdated log id");
    }
}
