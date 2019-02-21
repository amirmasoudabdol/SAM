//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENTSETUP_H
#define SAMPP_EXPERIMENTSETUP_H

#include <vector>
#include <iostream>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

enum ExperimentType {
    FixedModel,
    RandomModel,
    LatentModel
};

enum TestMethod {
    _tTest
};


/**
 \brief Define a class for ExperimentSetup.
 
 ExperimentSetup contains the necessary parameters for initiating and generating
 the data needed for the Experiment.
 */
class ExperimentSetup {


public:
    
    ExperimentType experimentType;
    TestMethod testMethod;
    
    int nc = 1;     ///< Number of experimental conditions, e.g., treatment 1, treatment 2.
    int nd = 3;     ///< Number of _dependent variables_ in each experimental condition.
    int ni = 0;     ///< Number of items for each latent variable, if `isFactorModel` is `true`.
    int ng;         ///< \brief Total number of groups
                    ///< Always calculated as \f$n_g = n_c * n_d\f$, unless the simulation contains latent variables, \f$n_g = n_c * n_d * n_i\f$
    int nrows;
    int nobs;       ///< Number of observations in each group

    double alpha = 0.05;
//    std::vector<std::string> group_names;
//    std::vector<std::string> dv_names;

    std::vector<int>    true_nobs;
    std::vector<double> true_means;
    std::vector<double> true_sds;
    
    ExperimentSetup() = default;
    
    ExperimentSetup(json& config);    
    
    ~ExperimentSetup() = default;

    void setNObs(int n_obs) {
        std::fill(true_nobs.begin(), true_nobs.end(), n_obs);
    };
    void setNObs(std::vector<int>& n_obs_v) {
        true_nobs = n_obs_v;
    }

    // Multivariate Experiments
    bool isCorrelated = false;
    double cov = 0;
    std::vector<std::vector<double>> true_sigma;
    
    // Latent Experiments
    std::vector<double> factorLoadings;                 ///< \f$\lambda\f$
    std::vector<double> errorSD;
    std::vector<std::vector<double>> errorCov;          ///<
    
    
};

#endif //SAMPP_EXPERIMENTSETUP_H
