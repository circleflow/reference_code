
#ifndef UTILITY_SMART_PTR_H_
#define UTILITY_SMART_PTR_H_

#ifdef ENV_C11
#include <memory>

using std::shared_ptr;
using std::weak_ptr;

#else

#include "boost/smart_ptr.hpp"

using boost::shared_ptr;
using boost::weak_ptr;

#endif

template <class T>
shared_ptr<T> persistent_shared(const T &t)
{
    shared_ptr<T> *psp = new shared_ptr<T>;
    psp->reset(new T(t));
    return *psp;
}

#endif /* UTILITY_SMART_PTR_H_ */
