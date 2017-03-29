
#ifndef UTILITY_ERROR_H_
#define UTILITY_ERROR_H_

#include "base_type.h"

#include <string>
using std::string;
#include <exception>
using std::exception;


namespace UTILITY {

    /* error process
     * > exception based error process
     * > macros defined to help to throw an error, catch an error...
     */

    namespace ERR {

        void printf(const char *fmt,...)
             __attribute__ ((format (printf, 1, 2)));

        //log, returns a unique log id, for further query
        int log(const string &);
        string get_log(int error_id);

        void terminate(const char *file, int line, const string &str);

        //printf into a string container, as a helper utility
        string _str_printf(void);
        string _str_printf(const char *fmt,...)
               __attribute__ ((format (printf, 1, 2)));

    }

    class EXP_ERROR : public exception
    {
    public:
        EXP_ERROR(const string &info);
        virtual ~EXP_ERROR() throw();

        void append(const string &info);
        const char *what() const throw ();

    private:
        string info;
    };

}

#define STR_PRINTF  UTILITY::ERR::_str_printf
#define ERR_PRINTF  UTILITY::ERR::printf
#define ERR_LOG     UTILITY::ERR::log

//classify error into recoverable(roll back by exception rewind) and unrecoverable(terminate)

//throw out an error, with detail code position info
#define ERROR(args...) \
        {\
            ERR_PRINTF("\r\n ERROR:%s:%s:%d",__FILE__,__FUNCTION__,__LINE__);\
            throw UTILITY::EXP_ERROR(STR_PRINTF(args));\
        }

//terminate, with detail code position info
#define TERMINATE(args...) \
            ERR::terminate(__FILE__,__LINE__,STR_PRINTF(args).c_str())

//assert
//no ERR_PRINTF, it may lead to recursive call, for example, if anything wrong in mutex
#define ASSERT(expr) \
        try {\
            if(!(expr)) {\
                TERMINATE();\
            }\
        }\
        catch(...) {\
            TERMINATE();\
        }


//assert expression value as zero
#define ASSERT_OK(expr) \
        try {\
            int rv = expr;\
            if(0 != rv) {\
                TERMINATE();\
            }\
        }\
        catch(...) {\
            TERMINATE();\
        }

//ensure
#define ENSURE(expr, args...) \
        if(!(expr)) {\
            ERR_PRINTF("\r\n ERROR:%s:%s:%d \r\n %s",__FILE__,__FUNCTION__,__LINE__,#expr);\
            throw UTILITY::EXP_ERROR(STR_PRINTF(args));\
        }

//ensure expression value as zero
#define ENSURE_OK(expr, args...) \
        {\
            int rv = expr;\
            if(0 != rv)  {\
                ERR_PRINTF("\r\n ERROR:%s:%s:%d \r\n %s, returned %d",__FILE__,__FUNCTION__,__LINE__,#expr,rv);\
                throw UTILITY::EXP_ERROR(STR_PRINTF(args));\
            }\
        }

//terminate any exception(error), returns with an error log id for further query
#define EXP_FREE_START \
    try {

#define EXP_FREE_END \
    }\
    catch (UTILITY::EXP_ERROR &e) {\
        return ERR_LOG(e.what());\
    }\
    catch (exception &e) {\
        ERR_PRINTF("\r\n ERROR:%s:%s:%d, STL:%s.",__FILE__,__FUNCTION__,__LINE__,e.what());\
        return ERR_LOG(e.what());\
    }\
    catch (...) {\
        ERR_PRINTF("\r\n ERROR:%s:%s:%d,unknown exception.",__FILE__,__FUNCTION__,__LINE__);\
        return ERR_LOG("unknown exception");\
    }

//terminate any exception(error), silently
#define EXP_FREE_END_NR \
    }\
    catch (UTILITY::EXP_ERROR &e) {\
    }\
    catch (exception &e) {\
        ERR_PRINTF("\r\n ERROR:%s:%s:%d, STL:%s.",__FILE__,__FUNCTION__,__LINE__,e.what());\
    }\
    catch (...) {\
        ERR_PRINTF("\r\n ERROR:%s:%s:%d,unknown exception.",__FILE__,__FUNCTION__,__LINE__);\
    }


//encapsulate any exception into type of EXP_ERROR
#define EXP_RECAP_START \
    try {

#define EXP_RECAP_END \
    }\
    catch (UTILITY::EXP_ERROR &e) {\
        throw;\
    }\
    catch (exception &e) {\
        ERR_PRINTF("\r\n ERROR:%s:%s:%d, STL:%s.",__FILE__,__FUNCTION__,__LINE__,e.what());\
        throw UTILITY::EXP_ERROR(e.what());\
    }\
    catch (...) {\
        ERR_PRINTF("\r\n ERROR:%s:%s:%d,unknown exception.",__FILE__,__FUNCTION__,__LINE__);\
        throw UTILITY::EXP_ERROR("unknown exception");\
    }



#endif /* UTILITY_ERROR_H_ */
