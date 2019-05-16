//
// Created by Amir Masoud Abdol on 2019-03-11.
//

#ifndef SAMPP_MAIN_H
#define SAMPP_MAIN_H

#include <mlpack/core.hpp>
#include <mlpack/methods/neighbor_search/neighbor_search.hpp>
#include <mlpack/methods/linear_regression/linear_regression.hpp>

#include <armadillo>

namespace sam {
    struct FLAGS {
        static bool PROGRESS;
        static bool DEBUG;
        static bool VERBOSE;
        static bool UPDATECONFIG;
    };
}

#endif //SAMPP_MAIN_H
