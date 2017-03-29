
#ifndef LUA_VM_H_
#define LUA_VM_H_

class LUA_VM {
public:

    typedef void (LIB_REGISTER) (void *lua_state);

    LUA_VM(const char *file, LIB_REGISTER *lib_reg);
    ~LUA_VM();

    int run (const char *func, const char *args);

private:
    void * lua_state;
};



#endif
