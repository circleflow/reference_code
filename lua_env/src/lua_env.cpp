
#include <utility/export/smart_ptr_u.h>
#include "lua_env.h"
#include "lua_vm.h"

#include <map>
using std::map;
using std::make_pair;
#include <string>
using std::string;

class LUA_ENV::IMPL {
public:

    IMPL(const char *default_path, LIB_REGISTER *lib)
    :dflt_path(default_path),
     dflt_lib(lib)
    {  }

    int run(const char *file_name, const char * func, const char * args)
    {
        string path = full_path(file_name);
        DB_VM::iterator it=db_vm.find(path);

        shared_ptr<LUA_VM> lua_vm;
        if(db_vm.end() == it) {
            lua_vm.reset( new LUA_VM(path.c_str(), dflt_lib) );
            db_vm.insert(make_pair(path, lua_vm));
        } else {
            lua_vm = it->second;
        }

        return lua_vm->run(func, args);
    }


    void reset(const char *file_name)
    {
        DB_VM::iterator it=db_vm.find(full_path(file_name));

        if(db_vm.end() != it) {
            db_vm.erase(it);
        }
    }

private:
    typedef map< string, shared_ptr<LUA_VM> > DB_VM;
    DB_VM db_vm;

    string dflt_path;
    LUA_ENV::LIB_REGISTER *dflt_lib;

    string full_path(const char *file_path)
    {
        string full=dflt_path + "/" + string(file_path);
        return full;
    }
};

LUA_ENV::LUA_ENV(const char *default_path, LIB_REGISTER *lib)
{
    pimpl = new IMPL(default_path, lib);
}

LUA_ENV::~LUA_ENV()
{
    delete pimpl;
}

int LUA_ENV::run(const char *file_name, const char * func, const char * args)
{
    return pimpl->run(file_name, func, args);
}

void LUA_ENV::reset(const char *file_name)
{
    pimpl->reset(file_name);
}
