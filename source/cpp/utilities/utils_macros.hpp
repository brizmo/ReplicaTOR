#ifndef UTILS_MACROS_HPP
#define UTILS_MACROS_HPP

#include <string>
#include <filesystem>
#include "utils_strings.hpp"

#define REPO_NAME "Reptora main_BMO Systems"

//#define LIKELY(x)      __builtin_expect(!!(x), 1)
//#define UNLIKELY(x)    __builtin_expect(x, 0)


#define RET( value){ if ( value == 1) return;}
#define MARK( ) std::cerr << "ERROR IN " << __FILE__ << "(@" << __LINE__ << "): " << '\n';
#define FXN_STAT( ) std::cout << "RUNNING " << __func__ << ". . ." << '\n';
#define ERR_OUT( msg) std::cerr << "ERROR IN " << __FILE__ << "(@" << __LINE__ << "): " << msg << '\n';
#define SHOW( var) std::cout << #var << " is " << var << std::endl;
#define LOG_OUT( msg) std::cout << msg << std::endl;
#define LOG_EXIT( msg){ std::cerr << "ERROR IN " << __FILE__ << "(@" << __LINE__ << "): " << msg << '\n';\
   std::cout << "EXITING . . ." << "\n";\
   exit( 1);\
}
#define NEWLINE( ){ std::cout<< std::endl;}

#define CHECK_CMD_ARGS( num){\
   if( argc < num + 1)\
   {\
      LOG_OUT("Incorrect number of cmd line args, expected "\
         + utils::TS( num)\
         + " only received "\
         + utils::TS( argc - 1)\
      );\
      exit( 0);\
   }\
}

#define HELLO( name){\
   LOG_OUT( name ": Good morning, Dave!\n");\
}

#define GOODBYE( name){\
   LOG_OUT( name ": If I don't see you again. . .");\
   LOG_OUT(". . . good afternoon. . .");\
   LOG_OUT(". . . good evening. . .");\
   LOG_OUT(". . . and good night. . .");\
}


#endif //SOURCE_UTILS_MACROS_HPP
