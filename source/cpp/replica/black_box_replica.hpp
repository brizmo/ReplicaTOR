#ifndef black_box_replica_HPP
#define black_box_replica_HPP

class BlackBoxReplica {

public:
    BlackBoxReplica() = default;
    ~BlackBoxReplica() = default;

    void initialize(int const id_in) {
        id_ = id_in;
    }

    void updateRoundStart() {
        timer_.setStart();
    }

    void updateRoundEnd(CostDataType const cost_in, CostDataType const cost_min_in) {
        prev_round_time_ = timer_.getRunningTimeElapsed();

        ++prev_round_number_;
        prev_round_cost_ = cost_in;
        cost_min_ = cost_min_in;

    }

    int id_ = -1;
    MyTime timer_;

    double prev_round_time_ = 0.;
    int prev_round_number_ = 0;

    CostDataType prev_round_cost_ = 0.;
    CostDataType cost_min_ = 0.;
};

#endif // black_box_replica_HPP