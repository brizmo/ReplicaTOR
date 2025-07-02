#ifndef UTILS_DISPLAY_HPP
#define UTILS_DISPLAY_HPP

#include "utils_macros.hpp"
#include "utils_strings.hpp"

namespace utils
{
    //TERMINAL DISPLAY AIDS//////////////////////////////////////////////////////
    /** Repeats symbol_ strings length_many times, used to create separation in
       terminal prin_outs
       @const std::string& symbol_ >> string to be repeated in banner
       @param const int length_ >> number of times to repeat symbol_
       */
    inline void print_banner( const std::string &symbol_, const int length_ = 80 )
    {
        std::string output = "";
        for ( int i = 0; i < length_; i++ )
            output += symbol_;
        LOG_OUT( output );
    }//--------------------------------------------------------------------------

    /** A log + banner
    Prints out: "\n" + msg_ + an 80 character banner of symbol_ +"\n"
       */
    inline void log_banner( std::string msg_, const std::string &symbol_ = "/" )
    {
        LOG_OUT( "" );
        print_banner( symbol_, 80 );
        LOG_OUT( msg_ );
        print_banner( symbol_, 80 );
        LOG_OUT( "" );
    }//--------------------------------------------------------------------------

    inline void goodbye()
    {
        log_banner(
            "Thank you for using " REPO_NAME
            "!\nWe hope you found the answer to your problem"
        );
    }//--------------------------------------------------------------------------

    /** prints any 1D array onto a single line, space delimited
       @param data_type* const m_ >> array of values to be printed
       @param const int c_ >> number of items in array
       */
    template< class data_type >
    inline void print_1D( data_type *const m_, const int c_ )
    {
        for ( int i = 0; i < c_; i++ ) {
            std::cout << TS( m_[ i ] ) << " ";
        }
        LOG_OUT( "" );
    }//--------------------------------------------------------------------------

    /** prints any 1D array onto a single line, space delimited
       @param data_type* const m_ >> array of values to be printed
       @param const int c_ >> number of items in array
       */
    template< class data_type >
    inline void print_1D_stack_with_id( data_type *const m_, const int c_ )
    {
        for ( int i = 0; i < c_; i++ ) {
            std::cout << "ID # " << i << " " << m_[ i ] << std::endl;
        }
        LOG_OUT( "" );
    }//--------------------------------------------------------------------------

    /** prints any 1D array onto a 2D grid, space delimited. Useful for printing
       out 1D state array of a problem like NQP into 2D chess board.
       @param data_type* const m_ >> array of values to be printed
       @param const int dim_ >> dimension of grid i.e. dim_ = sqrt( array size)
       */
    template< class data_type >
    inline void print_1D_as_grid( data_type *const m_, const int dim_ )
    {
        for ( int i = 0; i < dim_; i++ ) {
            for ( int j = 0; j < dim_; j++ )
                std::cout << int( m_[ i*dim_ + j ] ) << " ";
            NEWLINE();
        }

    }//--------------------------------------------------------------------------

    /** prints any 2D array, each row printed on 1 line, space delimited
       @param data_type** const m_ >> 2D array of values to be printed
       @param const int r_ >> number of rows in array ( 1st dim)
       @param const int c_ >> number of columns in array (2nd dim)
       */
    template< class data_type >
    inline void print_2D( data_type **const m_, const int r_, const int c_ )
    {
        for ( int i = 0; i < r_; i++ ) {
            for ( int j = 0; j < c_; j++ )
                std::cout << TS( m_[ i ][ j ] ) << " ";
            LOG_OUT( "" );
        }
        LOG_OUT( "" );
    }//--------------------------------------------------------------------------

    /** prints any 2D array, each row printed on 1 line, space delimited
       @param data_type** const m_ >> 2D array of values to be printed
       @param const int r_ >> number of rows in array ( 1st dim)
       @param const int c_ >> number of columns in array (2nd dim)
       */
    template< class data_type >
    inline void print_1D_as_2D( data_type *const m_, const int r_, const int c_ )
    {
        for ( int i = 0; i < r_; i++ ) {
            for ( int j = 0; j < c_; j++ )
                std::cout << m_[ i*c_ + j ] << " ";
            LOG_OUT( "" );
        }
        LOG_OUT( "" );
    }//--------------------------------------------------------------------------


    /**
     *
     * @tparam input_type
     * @tparam output_type
     * @param m_
     * @param c_
     */
    template< class input_type, class output_type >
    inline void print_1D_cast( void *const m_, const int c_ )
    {
        input_type *cast = ( input_type* ) m_;
        for ( int i = 0; i < c_; i++ ) {
            std::cout << ( output_type ) cast[ i ] << " ";
        } // end for i
        LOG_OUT( "" );
    }//--------------------------------------------------------------------------
};

#endif //SOURCE_UTILS_DISPLAY_HPP
