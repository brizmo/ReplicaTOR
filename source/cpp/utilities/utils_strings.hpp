#ifndef UTILS_STRINGS_HPP
#define UTILS_STRINGS_HPP

#include <string>

namespace utils
{
    //STRING UTILS///////////////////////////////////////////////////////////////
    /** strips string from leading and trailing whitespaces
       @param const std::string& str_ >> input string
       @param const std::string& whitespace_ >> a string of all chars to be
          stripped, by default set to whitespaces " \t\n"
       @return stripped string
       */
    inline std::string strip(
        const std::string &str_,
        const std::string &whitespace_ = " \t\n"
    )
    {
        const auto strBegin = str_.find_first_not_of( whitespace_ );
        if ( strBegin == std::string::npos )
            return ""; // no content

        const auto strEnd = str_.find_last_not_of( whitespace_ );
        const auto strRange = strEnd - strBegin + 1;

        return str_.substr( strBegin, strRange );
    }

    /** returns the substring before the specified delimeter. in_ will have
       substring up until the first delimeter deleted.
       Example: in_ = "hello;john", delim_ = ";"" >> returns "hello", in_ will be
       changed to "john"
       @param std::string &in_ >> input string, will be modified by this function
       @param std::string delim_ >> delimeter to search for
       @return substring before delimeter, in_ will be modifie
       */
    inline std::string pop_delim( std::string &in_, const std::string delim_ )
    {
        size_t pos = 0;
        pos = in_.find( delim_ );
        std::string sub = in_.substr( 0, pos );
        in_.erase( 0, pos + delim_.length() );

        return sub;
    }//--------------------------------------------------------------------------

    inline std::string get_filename( std::string in_ )
    {
        size_t pos = 0;
        pos = in_.find_last_of( "/" );
        in_.erase( 0, pos + 1 );

        return in_;
    }//--------------------------------------------------------------------------


    /** ToString(), converts any data type into a string
       @param const data_type input_ >> input to be converted to string
       @return input_ converted to string format
       */
    template< class data_type >
    inline std::string TS( const data_type input_ )
    { return std::to_string( input_ ); }//-----------------------------------------
};

#endif //SOURCE_UTILS_STRINGS_HPP
