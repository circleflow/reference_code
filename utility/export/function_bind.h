
#ifndef UTILITY_FUNCTION_BIND_H_
#define UTILITY_FUNCTION_BIND_H_


#ifdef ENV_C11
#include <functional>

using std::function;
using std::bind;

#define placeholders std::placeholders

#else

#include "boost/function.hpp"
#include "boost/bind.hpp"

using boost::function;
using boost::bind;

#define placeholders boost::placeholders

#endif

namespace UTILITY {

    class SCOPE_GUARD {
    public:
        typedef function<void (void)> OP;

        SCOPE_GUARD(const OP &_op)
        :op(_op)
        { }

        ~SCOPE_GUARD()
        { op(); }

    private:
        OP op;
    };

}

#endif /* UTILITY_FUNCTION_BIND_H_ */
