
#ifndef LUA_ENV_H_
#define LUA_ENV_H_

class LUA_ENV {
public:
    typedef void (LIB_REGISTER) (void *lua_state);

    LUA_ENV(const char *default_path, LIB_REGISTER *lib);
    ~LUA_ENV();

    int  run(const char *file_name,  const char * func, const char * args);

    void reset(const char *file_name);

private:
    class IMPL;
    IMPL *pimpl;
};

#endif
