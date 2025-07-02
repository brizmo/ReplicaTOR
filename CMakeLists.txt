cmake_minimum_required(VERSION 3.27)
project(ReplicaTOR)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -mfma -mavx -mfpmath=both -fopenmp -mpopcnt -mno-avx256-split-unaligned-load -mavx2 -funroll-loops -ffp-contract=fast -ftree-loop-if-convert -ftree-vectorize  -lstdc++ -lpthread -march=native -mtune=native -std=c++23 -O3 -fPIC")

add_subdirectory(external/eigen-3.4/)

include_directories(external/pcg-cpp-0.98/include/)

include_directories(source/cpp/defs)
include_directories(source/cpp/utilities)
include_directories(source/cpp/controller)
include_directories(source/cpp/model)
include_directories(source/cpp/replica)
include_directories(source/cpp/engine)


add_executable(ReplicaTOR solvers/solve_k_medoids.cpp
        source/cpp/replica/B_Q_cardinality_replica.hpp
        source/cpp/replica/B_Q_cardinality_replica.cpp
        source/cpp/defs/eigen_typedef.hpp
        source/cpp/defs/master_defs.hpp
        source/cpp/model/B_Q_cardinality_model.hpp
        source/cpp/utilities/my_time.hpp
        source/cpp/engine/B_Q_cardinality_engine.cpp
)

target_link_libraries (ReplicaTOR Eigen3::Eigen)
