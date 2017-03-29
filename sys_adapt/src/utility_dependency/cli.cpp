
extern "C" {

    typedef unsigned long (*cmd_proc_type) (char *, int *, char *);
    typedef void (*cmd_help_type) (void);

    unsigned long dbg_link (char *module_name,
                            cmd_help_type help_proc,
                            cmd_proc_type cmd_proc);

    int dbg_printf (const char *fmt,...);

    unsigned long dbg_scanstr (char *cmd, int *idx, char *result);
}

static
void cmd_output (const char *str)
{
    dbg_printf(str);
}

#include "utility/export/error.h"
#include "utility/export/cmd.h"
#include "utility/export/_dependency.h"
using namespace UTILITY;

#include <list>
using std::list;
#include <cstring>
using std::strcpy;

/* there is the gap of type between the CMD::HANDLER and cmd_proc_type, as well as CMD::HELPER and cmd_help_type
 * to solve this, a number of proxy defined, i.e. a number of function entity of cmd_proc_type and cmd_help_type
 * each proxy entity mapping to a corresponding CMD entity of CMD::HANDLER or CMD::HELPER during cmd link
*/

#include <map>
using std::map;
using std::pair;
using std::make_pair;


typedef map<cmd_proc_type, CMD::HANDLER> DB_HANDLER;
static DB_HANDLER db_handler;

typedef map<cmd_help_type, CMD::HELPER> DB_HELPER;
static DB_HELPER db_helper;

typedef list<cmd_proc_type> DB_PTR_PROC;
static
DB_PTR_PROC & get_db_ptr_proc()
{
    static DB_PTR_PROC db;
    return db;
}

typedef list<cmd_help_type> DB_PTR_HELP;
static
DB_PTR_HELP & get_db_ptr_help()
{
    static DB_PTR_HELP db;
    return db;
}

static
void cmd_link (const string &key, const CMD::HANDLER &handler, const CMD::HELPER &helper)
{
    ASSERT(get_db_ptr_proc().size()>0);
    ASSERT(get_db_ptr_help().size()>0);

    cmd_proc_type ptr_proc = get_db_ptr_proc().front();
    cmd_help_type ptr_help = get_db_ptr_help().front();

    db_handler.insert(make_pair(ptr_proc, handler));
    db_helper.insert(make_pair(ptr_help, helper));

    get_db_ptr_proc().pop_front();
    get_db_ptr_help().pop_front();

    char key_copy[20];

    ASSERT(sizeof(key_copy)>key.size());
    strcpy(key_copy, key.c_str());

    ASSERT(0 == dbg_link(key_copy, ptr_help, ptr_proc));
}

static
CMD::INPUTS convert(char *str, int *idx, char *cmd)
{
    CMD::INPUTS inputs;

    inputs.push_back(string(cmd));

    static char sub_cmd[256];
    while (0 == dbg_scanstr( str, idx, sub_cmd )) {
        inputs.push_back(string(sub_cmd));
    }

    return inputs;
}

//template of cmd function proxy, the ID will be used to generate multiple types
template<int ID>
class CMD_PROXY {
public:
    CMD_PROXY()
    {
        get_db_ptr_proc().push_back(proc_cb);
        get_db_ptr_help().push_back(help_cb);
    }

private:
    static
    unsigned long proc_cb (char *str, int *idx, char *cmd)
    {
        EXP_FREE_START;
        db_handler[proc_cb](convert(str, idx, cmd));
        EXP_FREE_END_NR;

        //to ensure the cli prompt appears at the bening of line, otherwise it will follow the end of cmd outputs
        cmd_output("\r\n");

        return 1;
    }

    static
    void help_cb (void)
    {
        EXP_FREE_START;
        db_helper[help_cb]();
        EXP_FREE_END_NR;
    }
};

#define LINENAME_CAT(name, line) name##line
#define LINENAME(name, line) LINENAME_CAT(name, line)

#define CMD_PROXY_INSTANCE \
    static CMD_PROXY<__LINE__> LINENAME(cmd_proxy,__LINE__);

//defines as much as need
CMD_PROXY_INSTANCE;
CMD_PROXY_INSTANCE;
CMD_PROXY_INSTANCE;
CMD_PROXY_INSTANCE;
CMD_PROXY_INSTANCE;
CMD_PROXY_INSTANCE;
CMD_PROXY_INSTANCE;
CMD_PROXY_INSTANCE;
CMD_PROXY_INSTANCE;
CMD_PROXY_INSTANCE;
CMD_PROXY_INSTANCE;
CMD_PROXY_INSTANCE;

namespace SYS_ADAPT {
    void cli_init(void)
    {
        cmd_set_op(cmd_link, cmd_output);
    }
}
