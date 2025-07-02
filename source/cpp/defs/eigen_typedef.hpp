//
// Created by brizmo (Mohammad Bagherbeik) on 6/23/25.
//

#ifndef REPLICATOR_EIGEN_TYPEDEF_HPP
#define REPLICATOR_EIGEN_TYPEDEF_HPP

#include <Eigen/Dense>

// stick to standard IEEE floats 32/64 bit

///< data type used to store problem definition coefficients for cost/constraints
typedef float CoeffDataType;

///< data type used to store cache data (e.g. local/constraint fields)
typedef float CacheDataType;

///< data type used for intermediate calculations
typedef float CalcDataType;

///< data type used for storing final cost function value
typedef float CostDataType;



template<class D_TYPE>
using RowMat = Eigen::Matrix<D_TYPE, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

template<class D_TYPE>
using ColMat = Eigen::Matrix<D_TYPE, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>;

template<class D_TYPE>
using RowVec = Eigen::Matrix<D_TYPE, 1, Eigen::Dynamic, Eigen::RowMajor>;

template<class D_TYPE>
using ColVec = Eigen::Matrix<D_TYPE, Eigen::Dynamic, 1, Eigen::ColMajor>;

#endif //REPLICATOR_EIGEN_TYPEDEF_HPP
