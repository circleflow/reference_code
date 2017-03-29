#include "utility/export/cmd.h"
#include "utility/export/error.h"

#include "lua_env/export/lua_env.h"
#include "lua_c_api/export/circle_flow.h"

using namespace UTILITY;

#define CMD_KEY      "cflow"
#define SCRIPT_PATH  "/cflow"

/* cli cmd for circle flow functionality, based on lua script */

static
void cmd_handler(const CMD::INPUTS &inputs) {
    EXP_FREE_START;

    size_t size = inputs.size(), index = 0;

    ENSURE(size > index);

    static LUA_ENV lua(SCRIPT_PATH, CIRCLE_FLOW_LUA_C::register_lib);

    if ("reset" == inputs[index]) {

        ENSURE(size > (++index));
        lua.reset(inputs[index].c_str());

    } else {

        string file(inputs[index]);
        string paras;

        //concatenate all parameters into single string
        while (size > (++index)) {
            paras += inputs[index];
            paras += string(" ");
        }

        lua.run(file.c_str(), "main", paras.c_str());

    }

    EXP_FREE_END_NR;
}

static
void cmd_helper(void) {
    static const char *help =
            "run/reset lua script file, under path of  "SCRIPT_PATH" \r\n"
            "to run:   prompt> script_name arguments....    \r\n"
            "to reset: prompt> reset script_name            \r\n";

    CMD::printf(help);

    cmd_handler(CMD::INPUTS(1, "help"));
}

static
CMD::AUTO_LINK __cmd(CMD_KEY, cmd_handler, cmd_helper);

