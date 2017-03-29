
#ifndef TEST_HELPER_H_
#define TEST_HELPER_H_

#include "gtest/gtest.h"
#include "opmock.h"

#define EXPECT_VERIFY_MOCK()  \
      opmock_test_verify(); \
      if (0 != opmock_get_number_of_errors_no_order()) \
      { \
        printf("\nOpmock error messages:\n"); \
        opmock_print_error_messages(); \
        ADD_FAILURE() << "VerifyMock failed \n"; \
      }


#include <stdio.h>
#include <sstream>
using std::ostream;
using std::ostringstream;
using std::endl;
#include <algorithm>
using std::for_each;
using std::sort;

template<typename T> class DUMP_VAL {
public:
    DUMP_VAL(ostream &_os) : os(_os) {}
    void operator () (const T& element)
    {
        os<<element<<", ";
    }

    ostream &os;
};

template<>
class DUMP_VAL<unsigned char> {
public:
    DUMP_VAL(ostream &_os) : os(_os)
    {
        os.width(2);
        os.fill('0');
        os.setf (std::ios::hex , std::ios::basefield);
        os.setf (std::ios::right , std::ios::adjustfield);
        os.unsetf (std::ios::showbase);
    }

    void operator () (const unsigned char & u8)
    {
        os<<(unsigned short)u8<<", ";
    }

    ostream &os;
};

template<typename C> void dump_container(ostream &os, const C &cont)
{
    for_each(cont.begin(), cont.end(), DUMP_VAL<typename C::value_type>(os));
}

template<typename T> int cmp_type(void *_expect, void *_actual, const char * name, char *buffer)
{
    T *expect = static_cast<T *> (_expect);
    T *actual = static_cast<T *> (_actual);

    if( (*expect) != (*actual)) {

        ostringstream oss;
        oss<<"parameter "<<name<<" expect: "<<*expect<<", actual: "<<*actual<<endl;
        snprintf(buffer, OP_MATCHER_MESSAGE_LENGTH, "%s", oss.str().c_str());

        return 1;
    }

    return 0;
}

template<typename C> int cmp_container(void *_expect, void *_actual, const char * name, char *buffer)
{

    C *expect = static_cast<C *> (_expect);
    C *actual = static_cast<C *> (_actual);

#define DUMP_EXPECT_ACTUAL \
    {\
        ostringstream oss;\
        oss<<"parameter "<<name<<" expect: ";\
        dump_container(oss, *expect);\
        oss<<" actual: "<<endl;\
        dump_container(oss, *actual);\
        oss<<endl;\
        snprintf(buffer, OP_MATCHER_MESSAGE_LENGTH, "%s", oss.str().c_str());\
    }

    if(expect->size() != actual->size()) {
        DUMP_EXPECT_ACTUAL;
        return 1;
    }

    if( (*expect) != (*actual)) {
        // try another way
        C expect_copy(*expect);

        for(typename C::iterator it=actual->begin(); it!=actual->end(); it++) {
            typename C::iterator it_find = find(expect_copy.begin(), expect_copy.end(), *it);
            if(it_find == expect_copy.end()) {
                DUMP_EXPECT_ACTUAL;
                return 1;
            } else {
                expect_copy.erase(it_find);
            }
        }
    }

#undef DUMP_EXPECT_ACTUAL

    return 0;
}



#endif  //TEST_HELPER_H_
