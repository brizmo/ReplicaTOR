#ifndef UTILS_FILE_IO_HPP
#define UTILS_FILE_IO_HPP

#include "utils_display.hpp"
#include "utils_macros.hpp"
#include "utils_strings.hpp"


namespace utils
{
    //FILE IO////////////////////////////////////////////////////////////////////
    inline std::string get_file_name_no_path_no_ext ( const std::string &full_name_ )
    {
        std::string no_path =
                full_name_.substr ( full_name_.find_last_of ( "\\/" ) + 1 );
        std::string no_ext =
                no_path.substr ( 0, no_path.find_last_of ( "." ) );

        return no_ext;
    }//--------------------------------------------------------------------------


    /** checks if file name_ exists
       @param const std::string& name_ >> passes in the file name by reference
       @return Returns true if file exists and false if it doesn't
       */
    inline bool file_exists ( const std::string &name_ )
    {
        struct stat buffer;
        return ( stat ( name_.c_str (), &buffer ) == 0 );
    }

    /** checks if file exists and exits program if it doesn't
       @param const std::string& name_ >> passes in the file name by reference
       */
    inline void file_check_exit ( const std::string &name_ )
    {
        if ( !file_exists ( name_ ) ) LOG_EXIT( "FILE " + name_ + " DOES NOT EXIST!" );
        LOG_OUT( "FILE " + name_ + " EXISTS!" )
    } //-------------------------------------------------------------------------

    template< class wb_type >
    inline void read_file_2D_eigen (
            RowMat<wb_type> &matrix_in,
            const int lines_,
            const int ipl_,
            const std::string w_file_
    )
    {
        file_check_exit ( w_file_ );

        //cant allocate extremely large arrays have to split up into 2D
        matrix_in.resize(lines_, ipl_);


        std::ifstream wStream ( w_file_ );
        std::string line;
        for ( int i = 0; i < lines_; i++ ) {
            std::getline ( wStream, line ); //get row
            std::istringstream lineStream ( line ); //conv to stringstream
            for ( int j = 0; j < ipl_; j++ ) {
                wb_type tf;
                lineStream >> tf;
                matrix_in(i, j) = tf;
            }//end for j

        }//endfor i

    } //-------------------------------------------------------------------------

    //PARAMETER FILE READS///////////////////////////////////////////////////////
    /** reads run time parameters from a file and loads them into a
       <key: string, value: string> map (hash table). Param file parameters can
       be store in any order. if a line starts with / it is counted as a comment.
       All other lines should follow the format: parameter_name value, with
       space used to separate them.
       @param std::map< std::string, std::string>& map_params_ >> map passed in by reference from a main() function to store params inside
       @param const std::string param_file_name_ >> parameter file name
       */
    inline void read_param_file_to_map (
            std::map<std::string, std::string> &map_params_,
            const std::string param_file_name_
    )
    {
        log_banner ( "LOADING PARAMETERS...", "\\" );

        file_check_exit ( param_file_name_ );
        std::ifstream fileStream ( param_file_name_ );
        std::string line;
        std::string name;
        std::string value;

        while ( std::getline ( fileStream, line ) ) {
            std::istringstream lineStream ( line ); //conv to stringstream
            lineStream >> name;
            //LOG_OUT( line);
            if ( name.find ( "/" ) == std::string::npos ) {
                lineStream >> value;
                map_params_[ name ] = value;
                LOG_OUT( name + " " + map_params_[ name ] );
            } // end if
        } // end while

        log_banner ( "FINISHED LOADING PARAMETERS...", "/" );

    }//--------------------------------------------------------------------------

    /** Returns parameter named by param_name_ from map_params_ in the correct
       data format from original string format inside the map. Refer to one of
       the included main.cpp files to see correct usage.
       @param std::map< std::string, std::string>& map_params_ >> map of all param <key, value> pairs
       @param const std::string& param_name_ >> name of param that needs to be extracted
       @return By specifying the correct data format in the template, the function returns the
       value of the requested parameter in the correct data type format
       */
    template< class data_type >
    inline data_type get_param (
            std::map<std::string, std::string> &map_params_,
            const std::string &param_name_
    )
    {
        if ( map_params_.count ( param_name_ ) == 0 ) {
            ERR_OUT( "PARAMETER " + param_name_ + " DOES NOT EXIST IN PARAM FILE" );
            LOG_OUT( "TERMINATING..." );
            exit ( 0 );
        }
        std::istringstream valStream ( map_params_[ param_name_ ] );
        data_type val;
        valStream >> val;
        return val;
    }//--------------------------------------------------------------------------
};
#endif // UTILS_FILE_IO_HPP
