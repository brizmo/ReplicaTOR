//
// Created by brizmo (Mohammad Bagherbeik) on 6/23/25.
//

#ifndef REPLICATOR_B_Q_CARDINALITY_MODEL_HPP
#define REPLICATOR_B_Q_CARDINALITY_MODEL_HPP

#include "master_defs.hpp"

class BQCardinalityModel {

public:
    BQCardinalityModel() = default;

    explicit BQCardinalityModel(std::map<std::string, std::string> &map_params_in) {
        initialize(map_params_in);
    }

    /**
     * Use to populate object if allocated dynamically
     * See Initialize() for list of parameters that must be inside param file
     * @param map_params_ >>
     *   parameter map built using utils::read_param_file_to_map
     */
    void initialize(std::map<std::string, std::string> &map_params_in)
    {
        initialize(
                utils::get_param<int>(map_params_in, "num_vars"),
                utils::get_param<int>(map_params_in, "num_k"),
                utils::get_param<CoeffDataType>(map_params_in, "B_scale_factor"), 
                utils::get_param<CoeffDataType>(map_params_in, "D_scale_factor"), 
                utils::get_param<std::string>(map_params_in, "problem_path"),
                utils::get_param<std::string>(map_params_in, "problem_name"),
                utils::get_param<CostDataType>(map_params_in, "cost_answer")
        );
    } //--------------------------------------------------------------------------------------------------------------------

    void initialize(
        int const num_vars_in,
        int const num_k_in,
        CoeffDataType const B_scale_fac_in,
        CoeffDataType const D_scale_fac_in,
        std::string const &problem_path_in,
        std::string const &problem_name_in,
        CostDataType const cost_ans_in
    ) {
        num_vars_ = num_vars_in;
        num_k_ = num_k_in;

        B_scaling_factor_ext = B_scale_fac_in;
        D_scaling_factor_ext = D_scale_fac_in;

        problem_path_ = problem_path_in;
        problem_name_ = problem_name_in;

        cost_ans_ = cost_ans_in;

        B_scaling_factor_ = B_scaling_factor_ext;
        D_scaling_factor_ = D_scaling_factor_ext;


        utils::read_file_2D_eigen<CoeffDataType>(
            D_mat_,
            num_vars_,
            num_vars_,
            problem_path_ + problem_name_ + ".d"
        );

        D_original_mat_ = D_mat_;

        generateBVector();
        scaleMatrices();


    } // ---------------------------------------------------------------------------------------------------------------

    void scaleMatrices() {
        B_vec_ *= B_scaling_factor_;

        D_mat_ *= -1.;
        D_mat_ *= D_scaling_factor_;
    }

    /**
     * @brief Generates cluster assignment based on K-Medoids
     */
    std::vector<int> generateAssignments(std::vector<int> const k_medoids_in)
    {
        std::vector<int> assignment;
        assignment.resize(num_vars_);

        CoeffDataType min_d;

        for (int i = 0; i < num_vars_; ++i) {
            assignment[i] = k_medoids_in[0];
            min_d = D_original_mat_(i, assignment[i]);

            for( int j = 1; j < num_k_; ++j ) {
                if (D_original_mat_(i, k_medoids_in[j]) < min_d) {
                    assignment[i] = k_medoids_in[j];
                    min_d = D_original_mat_(i, k_medoids_in[j]);
                }
            }
        }

        for (int j = 0; j < num_vars_; ++j){
            for (int i = 0; i < num_k_; ++i) {
                if (assignment[j] == k_medoids_in[i]) {
                    assignment[j] = i;
                    break;
                }
            }
        }

        return assignment;
    } // ---------------------------------------------------------------------------------------------------------------

    void generateAdjacencyMat()
    {
        utils::read_file_2D_eigen<int>(
                adjacency_mat_,
                num_vars_,
                num_vars_,
                problem_path_ + problem_name_ + ".adj"
        );

        num_total_edges_ = adjacency_mat_.sum() / 2;

        SHOW(num_total_edges_);
    }

    void getKValues(
        std::vector<int> const &assignment_in,
        float &k_inter_,
        float &k_,
        float &k_intra_
    )
    {
        generateAdjacencyMat();

        RowMat<float> EdgeCountByClass;
        EdgeCountByClass.setZero(num_k_, num_k_);

        RowVec<float> countByClass;
        countByClass.setZero(num_k_);

        // get count by class
        for ( int j = 0; j < num_vars_; ++j ) {
            ++countByClass[assignment_in[j]];
        } // end for j

        // get edge connectivity
        // get all nodes that belong to a class
        // get all nodes that belong to a class index above current
        for ( int i = 0; i < num_vars_; ++i ) {
            int ba_i = assignment_in[i];
            for ( int j = 0; j < num_vars_; ++j ) {
                int ba_j = assignment_in[j];
                if ( adjacency_mat_(i, j) == 1 ) {
                    if ( ba_i == ba_j ) {
                        EdgeCountByClass(ba_i, ba_j) += 0.5f;
                    }
                    else if ( ba_j > ba_i ) {
                        EdgeCountByClass(ba_i, ba_j) += 1.f;
                    } // end if
                } // end if
            } // end for j
        } // end for i

        k_ = float(num_total_edges_) / float(0.5f * num_vars_ * (num_vars_ - 1));
        k_intra_ = 0;
        k_inter_ = 0;

        for ( int i = 0; i < num_k_; ++i ) {
            float n_i = countByClass[ i ];
            if ( n_i > 1 ) {
                float denom1 = 0.5f * n_i * ( n_i - 1 );
                k_intra_ += EdgeCountByClass(i, i) / denom1;
            } // end if
            for ( int j = i + 1; j < num_k_; ++j  ) {
                float n_j = countByClass[ j ];
                float denom2 = (n_i + n_j) * (n_i + n_j - 1) - n_i *(n_i - 1) - n_j * (n_j - 1);
                denom2 *= 0.5f;
                k_inter_ += EdgeCountByClass(i, j) / denom2;
            } // end for j
        } // end for i

        k_intra_  /= float(num_k_);
        k_inter_ /= 0.5f * float(num_k_ * (num_k_ -1));

    }

private:
    friend class BQCardinalityReplica;
    friend class BQCardinalityEngine;

    int num_vars_ = 0;
    int num_k_ = 0;

    RowMat<CoeffDataType> D_original_mat_;

    RowMat<CoeffDataType> D_mat_;
    RowVec<CoeffDataType> B_vec_;

    int num_total_edges_ = 0;
    RowMat<int> adjacency_mat_;


    CostDataType cost_ans_ = std::numeric_limits<CostDataType>::max();

    std::string file_head;

    CoeffDataType D_scaling_factor_ext = 0.f;
    CoeffDataType B_scaling_factor_ext = 0.f;
    CoeffDataType D_scaling_factor_ = 0.f;
    CoeffDataType B_scaling_factor_ = 0.f;

    std::string problem_path_;
    std::string problem_name_;


    // OPT
    void generateBVector()
    {
        B_vec_.resize(num_vars_);
        B_vec_.setZero();

        for (int i = 0; i < num_vars_; ++i) {
            B_vec_(i) = D_mat_.row(i).sum();
        } // end for i
    }


};

#endif //REPLICATOR_B_Q_CARDINALITY_MODEL_HPP
