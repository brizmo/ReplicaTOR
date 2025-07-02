#ifndef parallel_tempering_controller_HPP
#define parallel_tempering_controller_HPP

#include "master_defs.hpp"
#include <random>

class ParallelTemperingController
{
public:

    int num_replicas_ = 0;

    CostDataType T_min_ = 0.;
    CostDataType T_max_ = 0.;

    int ladder_init_mode_ = 2;

    //! stores id of replica holding a temperature.
    //*! i.e. id_replicas_[3] --> id of replica holding temperature #3 (T_vec_[3]) */
    std::vector<int> id_replicas_;

    //! stores id of temperature assigned to a replica.
    //*! i.e. id_temps_[3] --> id of temperature assigned to replica #3 */
    std::vector<int> id_temps_;

    //! Stores temperature ladder in INCREASING order
    std::vector<CostDataType> temperatures_;

    //! Store inverse temperature ladder in DECREASING order
    std::vector<CostDataType> betas_;

    int iterations_per_round_ = 0;

    std::vector<int> scaled_iterations_per_round_;

    pcg32 rng_;

    //! Generates random number in range (0.0, 1.0), given an RNG object
    std::uniform_real_distribution<CostDataType> udist_;

    // -----------------------------------------------------------------------------------------------------------------
    /**
     * Default constructor
     */
    ParallelTemperingController() = default;

    /**
     * Default destructor
     */
    ~ParallelTemperingController() = default;

    /**
     * Used to initialize object instances
     * @param num_replicas_in
     * @param max_T_in
     * @param min_T_in
     * @param ladder_init_mode_in
     */
    void initialize(
        int const num_replicas_in,
        CostDataType const T_max_in,
        CostDataType const T_min_in,
        int const ladder_init_mode_in,
        int const iterations_per_round_in
    )
    {
        num_replicas_ = num_replicas_in;

        id_temps_.resize(num_replicas_);
        id_replicas_.resize(num_replicas_);
        temperatures_.resize(num_replicas_);
        betas_.resize(num_replicas_);

        initializeLoadBalancing(iterations_per_round_in);
        initializeRNG();
        initializeIDs();
        initializeLadder(T_max_in, T_min_in, ladder_init_mode_in);
    } //----------------------------------------------------------------------------------------------------------------

    void initializeLoadBalancing(int const iterations_per_round_in) {
        iterations_per_round_ = iterations_per_round_in;
        scaled_iterations_per_round_.resize(num_replicas_);

        for (int i = 0; i < num_replicas_; ++i) {
            scaled_iterations_per_round_[i] = iterations_per_round_;
        }
    }

    void initializeIDs() {
        for (int i = 0; i < num_replicas_; i++ ) {
            id_temps_[ i ] = i;
            id_replicas_[ i ] = i;
        }
    }


    void initializeLadder(
        CostDataType T_max_in,
        CostDataType T_min_in,
        int ladder_init_mode_in
    )
    {
        ladder_init_mode_ = ladder_init_mode_in;

        T_min_ = T_min_in;
        T_max_ = T_max_in;

        initializeIDs();

        temperatures_[0] = T_min_;
        temperatures_[num_replicas_ - 1] = T_max_;

        betas_[0] = 1.f / T_min_;
        betas_[num_replicas_ - 1] = 1.f / T_max_;

        switch(ladder_init_mode_) {
            case 0:
                generateLadderLinearT();
                break;
            case 1:
                generateLadderLinearBeta();
                break;
            case 2:
                generateLadderExponentialT();
                break;
            default:
                LOG_EXIT("Requested Ladder Init Mode does not exist!");
                break;
        }
    }

    void generateLadderLinearT() {
        CostDataType T_step = (T_max_ - T_min_) / (num_replicas_ - 1);

        for (int i = 1; i < num_replicas_ - 1 ; i++) {
            temperatures_[i] = temperatures_[i - 1] + T_step;
            betas_[i] = 1.f / temperatures_[i];
        } // end for i
    }

    void generateLadderLinearBeta() {
        CostDataType beta_step = (betas_[0] - betas_[num_replicas_ - 1]) / (num_replicas_ - 1);

        for (int i = 1; i < num_replicas_ - 1 ; i++) {
            betas_[i] = betas_[i - 1] - beta_step;
            temperatures_[i] = 1.f / betas_[i];
        } // end for i
    }

    void generateLadderExponentialT() {
        CostDataType T_step = pow(T_max_ / T_min_, 1.f / (num_replicas_ - 1));

        for (int i = 1; i < num_replicas_ - 1 ; i++) {
            temperatures_[i] = temperatures_[i - 1] * T_step;
            betas_[i] = 1.f / temperatures_[i];
        } // end for i
    }


    CostDataType getReplicaTemperature(const int replica_id_in)
    { return (temperatures_[id_temps_[replica_id_in]]); } //------------------------------------------------------------------------------

    int getTemperatureId(const int replica_id_in)
    { return  id_temps_[replica_id_in]; } //---------------------------------------------------------------------------------------

    int getReplicasIterationScalingFactor(const int replica_id_in)
    {
        int t_id = id_temps_[replica_id_in];
        return  scaled_iterations_per_round_[t_id];
    } //---------------------------------------------------------------------------------------

    bool checkSwap(
        CostDataType const &energy1_in,
        CostDataType const &energy2_in,
        CostDataType const &beta1_in,
        CostDataType const &beta2_in
    )
    {
        CostDataType accept = exp((beta1_in - beta2_in) * (energy1_in - energy2_in));
        if (accept >= 1.f) {
            return true;
        }
        else {
            return accept > udist_(rng_);
        } // end if
    } //----------------------------------------------------------------------------------------------------------------

    void sync(
        std::vector<CostDataType> const &energies_in,
        std::vector<double> const &round_times_in
    )
    {
        updateLoadBalanceScalingFactors(round_times_in);
        exchangeTemperatures(energies_in);
    } //----------------------------------------------------------------------------------------------------------------

    void exchangeTemperatures(std::vector<CostDataType> const &energies_in)
    {
        for (int i = 0; i < num_replicas_ - 1; i++ ) {
            int id1 = id_replicas_[i];
            int id2 = id_replicas_[i + 1];
            if (
                checkSwap(
                    energies_in[id1],
                    energies_in[id2],
                    betas_[i],
                    betas_[i + 1]
                )
            ) {
                id_replicas_[i] = id2;
                id_replicas_[i + 1] = id1;
                id_temps_[id2] = i;
                id_temps_[id1] = i + 1;
            }
        }
    } //----------------------------------------------------------------------------------------------------------------

    void updateLoadBalanceScalingFactors(std::vector<double> const &round_times_in)
    {
        std::vector<double> time_per_iteration;
        time_per_iteration.resize(num_replicas_);

        // round times are in replica order scaled iters are in T order, need to use id_replicas_
        for (int i = 0; i < num_replicas_; ++i) {
            int id = id_replicas_[i];
            time_per_iteration[i] = round_times_in[id] / scaled_iterations_per_round_[i];
        }

        double min_time_per_iteration = *std::min_element(time_per_iteration.begin(), time_per_iteration.end());

        for (int i = 0; i < num_replicas_; ++i) {
            scaled_iterations_per_round_[i] = (min_time_per_iteration / time_per_iteration[i]) * iterations_per_round_;
        }

    } //----------------------------------------------------------------------------------------------------------------


    /**
     * Initializes PCG32 random number generators
     */
    void initializeRNG()
    {
        pcg_extras::seed_seq_from< std::random_device > seed_source;
        rng_.seed(seed_source);
    } //--------------------------------------------------------------------------------------------------------------------

};

#endif // parallel_tempering_controller_HPP