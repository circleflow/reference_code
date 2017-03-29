
#ifndef EXPORT_CMD_H_
#define EXPORT_CMD_H_

#include <string>
using std::string;
#include <vector>
using std::vector;

#include "function_bind.h"

namespace UTILITY {

    namespace CMD {

        //cmd output
        void printf(const char *fmt, ...)
                __attribute__ ((format (printf, 1, 2)));

        //types
        typedef vector<string> INPUTS;

        typedef function< void (const INPUTS &) > HANDLER;
        typedef function< void (void) > HELPER;

        //link object, could be static object
        class AUTO_LINK {
        public:
            AUTO_LINK(const string &key, const HANDLER &, const HELPER &);
        };
    }
}



#endif /* EXPORT_CMD_H_ */
