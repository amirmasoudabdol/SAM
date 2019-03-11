//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENTSETUP_H
#define SAMPP_EXPERIMENTSETUP_H

#include <map>
#include <vector>
#include <iostream>

#include <armadillo>

#include "RandomNumberGenerator.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

/**
 * \brief      Specifying different type of expeirments.
 */
enum class ExperimentType {
    LinearModel,    ///< Linear Model, with or without covarinace
    LatentModel     ///< Latent Model or Structural Equation Model
};

/**
 * \brief      Specifying the significant testing method
 */
enum class TestMethod {
    TTest,           ///< T-test
    FTest            ///< F-test
};

const std::map<std::string, ExperimentType>
stringToExperimentType = {
    {"Linear Model", ExperimentType::LinearModel},
    {"Latent Model", ExperimentType::LatentModel}
};


/**
 \brief Define a class for ExperimentSetup.
 
 ExperimentSetup contains the necessary parameters for initiating and generating
 the data needed for the Experiment.
 */
class ExperimentSetup {
    
private:
    RandomNumberGenerator *RNGEngine;


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
    bool isNRandomized = false;
    
    double alpha = 0.05;
//    std::vector<std::string> group_names;
//    std::vector<std::string> dv_names;

    arma::Row<int>    true_nobs;
    arma::Row<double> true_means;
    arma::Row<double> true_vars;
    
    ExperimentSetup() = default;
    
    ExperimentSetup(json& config);    
    
    ~ExperimentSetup() = default;

    void setNObs(int n_obs) {
        std::fill(true_nobs.begin(), true_nobs.end(), n_obs);
    };
    void setNObs(std::vector<int>& n_obs_v) {
        true_nobs = n_obs_v;
    }
    
    void randomize_nObs();

    // Multivariate Experiments
    double cov = 0;
    arma::Mat<double> true_sigma;
    
    // Latent Experiments
    arma::Row<double> factorLoadings;                 ///< \f$\lambda\f$
    arma::Row<double> errorMeans;
    arma::Row<double> errorVars;
    arma::Mat<double> errorCov;          ///<
    
    
};

#endif //SAMPP_EXPERIMENTSETUP_H
