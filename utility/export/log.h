
#ifndef UTILITY_LOG_H_
#define UTILITY_LOG_H_

#include <string>
using std::string;

namespace UTILITY {

    //log info, and fetch info by log id
    namespace LOG {

        unsigned short log(const string &);

        string get_log(unsigned short log_id);
    }

}

////////////helper/////////

#define LOG(str) UTILITY::LOG::log(str)

#endif /* UTILITY_LOG_H_ */
