
#ifndef EXPORT_THREAD_U_H_
#define EXPORT_THREAD_U_H_

#include "base_type.h"
#include "function_bind.h"
#include <string>
using std::string;

namespace UTILITY {

    class THREAD {
    public:
        typedef function < void (void) > RUN_OP;

        THREAD(const string &name,
               UINT8 priority,   /*255 = highest, 1 = lowest*/
               UINT32 stack_size /*kbyte*/);
        ~THREAD();

        void run(const RUN_OP &op);

    private:
        class IMPL;
        IMPL *pimpl;

    };

}


#endif /* EXPORT_THREAD_U_H_ */
