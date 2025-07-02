#ifndef E_KBMO_HPP
#define E_KBMO_HPP

#include <omp.h>
#include "parallel_tempering_controller.hpp"
#include "B_Q_cardinality_model.hpp"
#include "B_Q_cardinality_replica.hpp"

class BQCardinalityEngine
{
public:
    BQCardinalityEngine() = default;
    ~BQCardinalityEngine() = default;

    explicit BQCardinalityEngine(
        BQCardinalityModel const &problem_in,
        std::map< std::string, std::string > &map_params_in
    );

    void initialize(
        BQCardinalityModel const &problem_in,
        std::map< std::string, std::string > &map_params_in
    );

    void initialize(
            BQCardinalityModel const &problem_in,
            int const round_limit_in,
            int const num_replicas_per_controller_in,
            int const num_controllers_in,
            int const num_cores_per_controller_in,
            CostDataType const T_max_in,
            CostDataType const T_min_in,
            int const ladder_init_mode_in,
            double time_limit_in
    );

    bool solve ();
    int executeRound();
    void print_best_Ks();
    std::vector<int> getCostMinState();

    /**
     * Returns run time between when timer was set after fdb was read from file
     * and until run() completed
     * @return >> time in seconds
     */
    double getRunTime()
    { return timer_.getTimeElapsed(); } //----------------

    int getCurrentRound()
    { return current_round; }

    CostDataType getCostMin();
    int getCostMinID();

    void gen_best_assignment();


protected:
    //VARIABLES/////////////////////////////////////////////////////////////////////////////////////////////////////////
    MyTime timer_;

    int dump_id = 0;

    BQCardinalityModel const *problem_;

    int num_replicas_per_controller_ = 32;
    int num_controllers_ = 1;
    int num_cores_per_controller_ = 32;
    int num_total_replicas_ = 32;
    int num_total_cores_ = 32;

    std::vector<ParallelTemperingController> pt_controllers_;
    std::vector<BQCardinalityReplica> replicas_;

    CostDataType prev_cost_min_ = 0;

    int cost_min_counter_ = 0;

    CostDataType cost_min_ = 0.;

    int iterations_per_round_ = 1000;
    int round_limit_ = 1000000;

    double time_limit_ = 0.;

    std::vector<int> t_id_;


    bool is_opt_ = false;
    std::vector<CostDataType> replica_cost_;
    std::vector<bool> replica_is_opt_;
    std::vector<std::vector<double>> round_times_;

    int totNumEdges = 0;

    // system state variables
    int ans_id = -1;
    int current_round = 0;

    //UTILITIES/////////////////////////////////////////////////////////////////////////////////////////////////////////
    void resizeData();
    void initializeReplicas();
    int executeSearch();

    void generateFoldedReplicaIDs();

    void initializeController(
        CostDataType const T_max_in,
        CostDataType const T_min_in,
        int const ladder_init_mode_in
    );

};

#endif // PBMO_E_HPP
