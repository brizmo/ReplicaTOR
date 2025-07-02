//
// Created by brizmo (Mohammad Bagherbeik) on 6/23/25.
//

#ifndef REPLICATOR_MY_TIME_HPP
#define REPLICATOR_MY_TIME_HPP

#include "master_defs.hpp"

/** @brief Class used to simplify timing calculations. Basically a stopwatch.

   @author Mohammad Bagherbeik
   @date July 2019
   */
class MyTime
{
public:
    //VARIABLES------------------------------------------------------------------
    std::chrono::time_point<std::chrono::high_resolution_clock> t_start_; ///< start time
    std::chrono::time_point<std::chrono::high_resolution_clock> t_end_; ///< end time

    double t_accum_ = 0;

    //CONSTRUCTORS---------------------------------------------------------------
    /** default constructor
       */
    MyTime() = default;

    /** default destructor
       */
    ~MyTime() = default;

    //UTILITIES------------------------------------------------------------------
    /** set start time of timer
       */
    void __attribute__ ((noinline)) setStart()
    {
        t_start_ = std::chrono::high_resolution_clock::now();
    }//--------------------------------------------------------------------------

    void __attribute__ ((noinline)) setEnd()
    {
        t_end_ = std::chrono::high_resolution_clock::now();
    }//--------------------------------------------------------------------------

    double getTimeElapsed( )
    {
        std::chrono::duration<double> elapsed = t_end_ - t_start_;

        return double(elapsed.count());
    }//--------------------------------------------------------------------------

    double getRunningTimeElapsed( )
    {
        auto t_now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = t_now - t_start_;

        return double(elapsed.count());
    }//--------------------------------------------------------------------------

    /** measures the amount of time passed from t_start until now and adds it
       to t_accum
       @return Time accumulated until now
       */
    double __attribute__ ((noinline)) accumulate()
    {
        t_end_ = std::chrono::high_resolution_clock::now();
        std::chrono::duration< double > elapsed = t_end_ - t_start_;

        t_accum_ += double(elapsed.count());

        return t_accum_;
    }


};

#endif //REPLICATOR_MY_TIME_HPP
