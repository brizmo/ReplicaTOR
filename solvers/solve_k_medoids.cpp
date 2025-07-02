//
// Created by brizmo (Mohammad Bagherbeik) on 6/24/25.
//
#include <omp.h>
#include "B_Q_cardinality_engine.hpp"

int main( int argc, char *argv[] ) {
    omp_set_num_threads( 64 );
    omp_set_nested(true);
    CHECK_CMD_ARGS(1);

    std::cout.precision(15);

    // load in path to problem files
    std::map< std::string, std::string > map_params;
    std::string param_file_name( argv[ 1 ] );

    utils::read_param_file_to_map( map_params, param_file_name );

    LOG_OUT("Building Model...\n");
    BQCardinalityModel k_medoid_model(map_params);

    LOG_OUT("Building Engine...\n");
    BQCardinalityEngine engine(k_medoid_model, map_params); // build

    LOG_OUT("Running Solver...\n");
    bool success = engine.solve();
    LOG_OUT(".....Solver Done...\n");

    LOG_OUT("Solver Results:")
    LOG_OUT(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    CostDataType cost_min = engine.getCostMin();
    double solve_time_in_seconds = engine.getRunTime();
    int rounds_executed = engine.getCurrentRound();
    SHOW(cost_min);
    SHOW(solve_time_in_seconds);
    SHOW(rounds_executed);
    LOG_OUT("");

    std::vector<int> best_state = engine.getCostMinState();

    LOG_OUT("K Medoid Indices:");
    for (auto &value : best_state) {
        std::cout << value << " ";
    }
    LOG_OUT("\n");

    std::vector<int> assignments;
    assignments = k_medoid_model.generateAssignments(best_state);

    LOG_OUT("Cluster Assignments (from 0 to K-1):");
    for (auto &value : assignments) {
        std::cout << value << " ";
    }
    LOG_OUT("\n");

	// comment out code for calculating k if no adjacency data file is available
    float k_inter, k, k_intra;
    k_medoid_model.getKValues(assignments, k_inter, k, k_intra);

    SHOW(k_inter);
    SHOW(k);
    SHOW(k_intra);
    LOG_OUT("\n");

    return 0;
}