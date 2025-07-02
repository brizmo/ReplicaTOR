#include "B_Q_cardinality_engine.hpp"

/**
 * Constructor that uses a parameter map read out from a file.
 * See utils::read_param_file_to_map
 * Will exit if it can't find all the required parameters
 * See Initialize for list of params that must be included in file
 * @param map_params_ >> the parameter map
 */
BQCardinalityEngine::BQCardinalityEngine(
    BQCardinalityModel const &problem_in,
    std::map< std::string, std::string > &map_params_in
)
{
    initialize(problem_in,map_params_in);
}//---------------------------------------------------------------------------------------------------------------------


void BQCardinalityEngine::initialize(
    BQCardinalityModel const &problem_in,
    std::map<std::string, std::string> &map_params_in
)
{
    initialize(
        problem_in,
        utils::get_param<int>(map_params_in, "round_limit"),
        utils::get_param<int>(map_params_in, "num_replicas_per_controller"),
        utils::get_param<int>(map_params_in, "num_controllers"),
        utils::get_param<int>(map_params_in, "num_cores_per_controller"),
        utils::get_param<CostDataType>(map_params_in, "T_max"),
        utils::get_param<CostDataType>(map_params_in, "T_min"),
        utils::get_param<int>(map_params_in, "ladder_init_mode"),
        utils::get_param<double>(map_params_in, "time_limit")
    );
} // -------------------------------------------------------------------------------------------------------------------

void BQCardinalityEngine::initialize(
        BQCardinalityModel const &problem_in,
        int const round_limit_in,
        int const num_replicas_per_controller_in,
        int const num_controllers_in,
        int const num_cores_per_controller_in,
        CostDataType const T_max_in,
        CostDataType const T_min_in,
        int const ladder_init_mode_in,
        double const time_limit_in
)
{
    problem_ = &problem_in;

    iterations_per_round_ = problem_->num_vars_ * problem_->num_k_;
    round_limit_ = round_limit_in;

    time_limit_ = time_limit_in;

    num_replicas_per_controller_ = num_replicas_per_controller_in;
    num_controllers_ = num_controllers_in;
    num_cores_per_controller_ = num_cores_per_controller_in;

    num_total_replicas_ = num_replicas_per_controller_ * num_controllers_;
    num_total_cores_ = num_controllers_ * num_cores_per_controller_;

    resizeData();

    // set fold ids of replicas
    generateFoldedReplicaIDs();

    initializeReplicas();

    initializeController(
        T_max_in,
        T_min_in,
        ladder_init_mode_in
    );

    prev_cost_min_ = getCostMin();

} // -------------------------------------------------------------------------------------------------------------------

void BQCardinalityEngine::resizeData() {
    pt_controllers_.resize(num_controllers_);
    replicas_.resize(num_total_replicas_);

    t_id_.resize(num_total_replicas_);

    replica_cost_.resize(num_total_replicas_);
    replica_is_opt_.resize(num_total_replicas_);

    round_times_.resize(num_controllers_);
    for (int i = 0; i < num_controllers_; ++i) {
        round_times_[i].resize(num_replicas_per_controller_);
    }
}

void BQCardinalityEngine::initializeController(
    CostDataType const T_max_in,
    CostDataType const T_min_in,
    int const ladder_init_mode_in
)
{
    for (int i = 0; i < num_controllers_; ++i) {
        pt_controllers_[i].initialize(
            num_replicas_per_controller_,
            T_max_in,
            T_min_in,
            ladder_init_mode_in,
            iterations_per_round_
        );
    }
} // -------------------------------------------------------------------------------------------------------------------


/**
 * Run PBMO with openmp multithreading
 * @return
 */
bool BQCardinalityEngine::solve()
{
    timer_.setStart();
    ans_id = executeSearch();

    timer_.setEnd();

    return ( ans_id >= 0 );
} //--------------------------------------------------------------------------------------------------------------------


/**
 * Runs a sweep across all replicas in folded sequence
 * @return >> true if any replica finds optimal answer and SPEED_MODE != 0
 *   and false if no answer found or SPEED_MODE == 0
 */
int BQCardinalityEngine::executeSearch ()
{
    int ans; // stores id of optimal replica or -1 if none ore opt

    for (current_round = 1; current_round < round_limit_; ++current_round) {

        ans = executeRound(); // run sweep

        if ( ans != -1 ) {
            return ans;
        } // end if
        else if (timer_.getRunningTimeElapsed() >= time_limit_) {
            break;
        }
    } // end for current_round

    return -1; // no replica found optimal answer
} //--------------------------------------------------------------------------------------------------------------------


/**
 * runs sweeps on all replicas using OpenMP
 * @param sweep_ current sweep number
 * @return id of first replcia that found optimal energy or -1 if not
 */
int BQCardinalityEngine::executeRound()
{
    // runs sweeps on replicas
    #pragma omp parallel for num_threads(num_total_cores_) default(none)
    for ( int r = 0; r < num_total_replicas_; ++r ) {
        // get folded engine # and replica id
        int engine = ( r / num_total_cores_ ) % num_controllers_;
        int r_id = engine * num_replicas_per_controller_ + t_id_[ r ];

        replica_is_opt_[r_id] = replicas_[r_id].executeRound(
            pt_controllers_[engine].getReplicaTemperature(t_id_[r]),
            pt_controllers_[engine].getReplicasIterationScalingFactor(t_id_[r])
        );

        round_times_[engine][t_id_[r]] = replicas_[r_id].getRoundTime();

        replica_cost_[r_id] = replicas_[r_id].getCost();
    } // end for r


    // loop through status bits of replicas to see if any reached optimum
    for ( int r = 0; r < num_total_replicas_; ++r ) {
        if (replica_is_opt_[r]) {
            return r;
        } // end if
    } // end for r


    for (int i = 0; i < num_controllers_; ++i) {
        pt_controllers_[i].sync(replica_cost_, round_times_[i]); // perform temperature swaps
    } // end for r


    CostDataType curr_cost_min = getCostMin();
    if ( curr_cost_min < prev_cost_min_ ) {
        prev_cost_min_ = curr_cost_min;
        cost_min_counter_ = 0;
    }
    else {
        ++cost_min_counter_;
    }
//    SHOW(cost_min_counter_);
    if (cost_min_counter_ == 10000){
        return getCostMinID();
    }


    return -1; // no replica is opt, return -1
} //--------------------------------------------------------------------------------------------------------------------



/**
 * Use multithread loop to initialize all replicas
 */
void BQCardinalityEngine::initializeReplicas()
{
    #pragma omp parallel for num_threads(num_total_cores_) default(none)
    for (int r = 0; r < num_total_replicas_; ++r) {
        replicas_[r].initialize(
            r,
            problem_
        );
    }
} //--------------------------------------------------------------------------------------------------------------------


/**
 *
 * @return minimum energy found among replicas
 */
CostDataType BQCardinalityEngine::getCostMin()
{
    CostDataType cost_min = replicas_[0].getCostMin();
    CostDataType cost_min_new;
    for ( int i = 1; i < num_total_replicas_; ++i ) {
        cost_min_new = replicas_[i].getCostMin();
        if (cost_min_new < cost_min) {
            cost_min = cost_min_new;
        }
    }

    return cost_min;
} //--------------------------------------------------------------------------------------------------------------------


int BQCardinalityEngine::getCostMinID()
{
    int r_min = 0;
    CostDataType cost_min = replicas_[0].getCostMin();
    CostDataType cost_min_new;
    for ( int i = 1; i < num_total_replicas_; ++i ) {
        cost_min_new = replicas_[i].getCostMin();
        if (cost_min_new < cost_min) {
            cost_min = cost_min_new;
            r_min = i;
        }
    }

    return r_min;
}

std::vector<int> BQCardinalityEngine::getCostMinState() {
    int r_id = getCostMinID();

    return replicas_[r_id].getCostMinState();

}


/**
 * generates replica temperature ids used for folding
 */
void BQCardinalityEngine::generateFoldedReplicaIDs()
{
    for ( int r = 0; r < num_total_replicas_; ++r ) {
        int fold_even_odd = (r / (num_total_cores_)) % 2;
        int fold_mul = (r / (num_total_cores_)) / 2;
        int r_fold_id = r % num_cores_per_controller_;
        int r_rel_id = fold_mul * num_cores_per_controller_ + r_fold_id;
        int id = (fold_even_odd == 0) ? r_rel_id : num_replicas_per_controller_ - 1 - r_rel_id;

        t_id_[r] = id;
    } // end for r
}