#include "B_Q_cardinality_replica.hpp"

/**
 * Use to populate object if allocated dynamically and for some reason you
 *   don't want to use map_params
 * @param id_ replica id
 * @param dim_ dimension of problem
 * @param E_ans_ optimal energy of problem
 * @param sweep_num_horizontal_ number of trials before PT swap is called
 * @param max_sweeps_ max number of sweeps to be performed
 * @param neuron_init_mode_ neuron initialization mode. 0 is diagonal Initialize, 1 is random
 * @param f_ pointer to 1D array containing flattened F matrix
 * @param d_ pointer to 1D array containing flattened D matrix
 * @param b_ pointer to 1D array containing flattened B matrix
 * @param f_trans_ pointer to 1D array containing flattened F_TRANS matrix
 * @param d_trans_ pointer to 1D array containing flattened D_TRANS matrix
 * @param window_size_
 */
void BQCardinalityReplica::initialize (
    int const id_in,
    BQCardinalityModel const *problem_in
)
{
    black_box_.initialize(id_in);

    problem_ = problem_in;

    cost_min_ = std::numeric_limits<CostDataType>::max();

    resizeData();

    initRNG();
    initializeState();

} //--------------------------------------------------------------------------------------------------------------------

//UTILITIES/////////////////////////////////////////////////////////////////

void BQCardinalityReplica::resizeData(){
    k_on_.resize(problem_->num_k_);
    k_on_min_.resize(problem_->num_k_);

    H_vec_.resize(problem_->num_vars_);
    H_vec_.setZero();
}

/**
 * initialize rng objects
 */
void BQCardinalityReplica::initRNG()
{
    pcg_extras::seed_seq_from<std::random_device> seed_source;
    rng_.seed (seed_source);
} //--------------------------------------------------------------------------------------------------------------------


/**
 * initialize state based on current value of neuron_init_mode and Initialize local fields and energies
 */
void BQCardinalityReplica::initializeState ()
{
    initializeKOn();
    initializeCounters();

    initializeHAndCost();
} //--------------------------------------------------------------------------------------------------------------------

/**
 * set state to a random permutation
 */
void BQCardinalityReplica::initializeKOnRandom()
{
    for (int i = 0; i < problem_->num_k_; ++i) {
        k_on_[i] = rng_() % problem_->num_vars_; // randomly add node to kon

        // check if node is a duplicate
        while (k_on_set_.find(k_on_[i]) != k_on_set_.end()) {
            k_on_[i] = rng_() % problem_->num_vars_;
        } // end while

        k_on_set_.insert(k_on_[i]); // add new node to set for fast checking
    } // end for i
} //--------------------------------------------------------------------------------------------------------------------

void BQCardinalityReplica::initializeKOn() {
    initializeKOnRandom();

//    for (int i = 0; i < problem_->num_k_; ++i) {
//        std::cout << k_on_[i] << " ";
//    }
//    std::cout << std::endl;
} //--------------------------------------------------------------------------------------------------------------------

void BQCardinalityReplica::initializeCounters ()
{
    counter_k_on_ = 0;

    i_on_ = k_on_[counter_k_on_]; // Initialize counter to first item in kon

    i_off_ = (i_on_ + 1) % problem_->num_vars_; // start from node right after i_on

    // if already in kon, repeat until new node is found
    while (k_on_set_.find(i_off_) != k_on_set_.end()) {
        i_off_ = (++i_off_) % problem_->num_vars_;
    }
} //--------------------------------------------------------------------------------------------------------------------

/**
 * initialize local fields and energy and set minimum state and energiy
 */
void BQCardinalityReplica::initializeHAndCost ()
{
    for ( int i = 0; i < problem_->num_vars_; ++i ) {
        for ( int j = 0; j < problem_->num_k_; ++j ) {
            //only check the bits that are on to save time
            H_vec_(i) += problem_->D_mat_(i, k_on_[j]);
        } // end for nk
    } // end for i

    for ( int i = 0; i < problem_->num_k_; ++i ) {
        cost_ += H_vec_(k_on_[i]);
    }

    cost_ /= 2.;

    for (int i = 0; i < problem_->num_k_; ++i) {
        cost_ += problem_->B_vec_(k_on_[i]);
    }

    for (int i = 0; i < problem_->num_vars_; ++i) {
        H_vec_(i) += problem_->B_vec_(i);
    } // end for i

    setMinState();
} //--------------------------------------------------------------------------------------------------------------------


/**
 * sets E_min to current E and copies current psi state into psi_min
 */
void BQCardinalityReplica::setMinState ()
{
    cost_min_ = cost_;
    std::copy_n(k_on_.begin(), k_on_.size(), k_on_min_.begin());
} //--------------------------------------------------------------------------------------------------------------------

/**
 * Run a sweep on the replica
 * @param beta_ beta value for trials
 * @param h_sweep_num_ number of trials
 * @return true if E_ans found, false otherwise
 */
bool BQCardinalityReplica::executeRound(
    CostDataType const T_in,
    int const iterations_per_round_in
)
{

    T_neg_ = -1. * T_in;
    iterations_per_round_ = iterations_per_round_in;

    black_box_.updateRoundStart();
    executeRoundSerialExchange();
    black_box_.updateRoundEnd(cost_, cost_min_);


//#if USE_MARGIN == 1
//    return ( E_min <= E_ans*SOFT_CON_MARGIN  );
//#else
    return (cost_min_ <= problem_->cost_ans_);
//#endif
} //--------------------------------------------------------------------------------------------------------------------


inline CalcDataType BQCardinalityReplica::calcDeltaCExchange(int const &i_on_in, int const &i_off_in)
{
    return
        H_vec_(i_off_in)
        - H_vec_(i_on_in)
        - problem_->D_mat_(i_on_in, i_off_in)
    ;
}

/**
 * Runs a sweep of 4-bit swap trials (kind of like 2W1H)
 * All flows/distances/weights are assumed to be symmetric for GQAP problems
 * Asymmetric problems require extra calcs that aren't included here
 */
void BQCardinalityReplica::executeRoundSerialExchange()
{
    CalcDataType delta_cost; // total change in energy including violation costs

    // attempt a total number of sweep_num_vertical trials, def value sweep_num_vertical = facs * ( locs )
    for ( int i = 0; i < iterations_per_round_; ++i ) {
        delta_cost = calcDeltaCExchange(i_on_, i_off_);

        // run trial
        if (T_neg_ * std::log(udist_(rng_)) > delta_cost) {
            cost_ += delta_cost; // update energy
            updateHExchange(i_on_, i_off_); // lf Update
            k_on_set_.insert(i_off_);
            k_on_set_.erase (i_on_);
            k_on_[counter_k_on_] = i_off_;

            if (cost_ < cost_min_) { // update E_min only if in a non-violating state
                setMinState();
            } // update E_min state

            counter_k_on_ = (++counter_k_on_) % problem_->num_k_;
            i_on_ = k_on_[counter_k_on_];
            i_off_ = i_on_;
        }
        else if ( i_off_ == i_on_ - 1 ) {
            counter_k_on_ = (++counter_k_on_) % problem_->num_k_;
            i_on_ = k_on_[counter_k_on_];
            i_off_ = i_on_;
        } // end if

        i_off_ = (++i_off_) % problem_->num_vars_;
        while (k_on_set_.find(i_off_) != k_on_set_.end() ) {
            i_off_ = (++i_off_) % problem_->num_vars_;
        }
    } // end for

} //--------------------------------------------------------------------------------------------------------------------

void BQCardinalityReplica::updateHExchange(int const &i_on_in, int const &i_off_in)
{
    H_vec_ += problem_->D_mat_.row(i_off_in) - problem_->D_mat_.row(i_on_in);
} //--------------------------------------------------------------------------------------------------------------------