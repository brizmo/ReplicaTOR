#ifndef REPLICATOR_B_Q_CARDINALITY_REPLICA_HPP
#define REPLICATOR_B_Q_CARDINALITY_REPLICA_HPP

#include "master_defs.hpp"
#include "B_Q_cardinality_model.hpp"
#include "black_box_replica.hpp"

#include <set>

class BQCardinalityReplica
{
public:
    BQCardinalityReplica() = default;
    ~BQCardinalityReplica() = default;

    void initialize(
        int const id_in,
        BQCardinalityModel const *problem_in
    );





    CostDataType getCost()
    { return cost_; } //----------------------------------------------------------------------------------------------------

    CostDataType getCostMin ()
    { return cost_min_; } //------------------------------------------------------------------------------------------------

    std::vector<int> getCostMinState() {
        return k_on_min_;
    }

    bool executeRound(
        CostDataType const T_in,
        int const iterations_per_round_in
    );

    void executeRoundSerialExchange();

    double getRoundTime()
    { return black_box_.prev_round_time_; }


protected:
    //VARIABLES/////////////////////////////////////////////////////////////////////////////////////////////////////////

    BlackBoxReplica black_box_;

    CostDataType T_neg_ = 0.0f;
    pcg32 rng_; ///< pointer to array of rng objects, 1 per replica

    std::uniform_real_distribution<CostDataType> udist_;


    BQCardinalityModel const *problem_ = nullptr;

    int iterations_per_round_ = 0;

    RowVec<CacheDataType> H_vec_;

    CostDataType cost_ = 0.f;
    CostDataType cost_min_ = 0.f;

    std::set< int > k_on_set_;
    std::vector<int> k_on_;
    std::vector<int> k_on_min_;

    int counter_k_on_ = 0;
    int i_on_ = 0;
    int i_off_ = 0;

    int* row_rcB = nullptr;

    //UTILITIES/////////////////////////////////////////////////////////////////////////////////////////////////////////

    void resizeData();
    void initializeCounters();
    void initRNG();
    void initializeState();
    void initializeKOn();
    void initializeKOnRandom();
    void initializeHAndCost();
    void setMinState();

    void updateHExchange(int const &i_on_in, int const &i_off_in);
    inline CalcDataType calcDeltaCExchange(int const &i_on_in, int const &i_off_in);

    void reset_state ();
};

#endif // REPLICATOR_B_Q_CARDINALITY_REPLICA_HPP

