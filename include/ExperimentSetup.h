//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENTSETUP_H
#define SAMPP_EXPERIMENTSETUP_H

#include <vector>
#include <iostream>



enum ExperimentType {
    FixedModel,
    RandomModel,
    FactorModel
};


/**
 \brief Define a class for ExperimentSetup.
 
 ExperimentSetup contains the necessary parameters for initiating and generating
 the data needed for the Experiment.
 */
class ExperimentSetup {


public:
    
    ExperimentType experimentType;
    
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

    std::vector<double> true_means;
    std::vector<double> true_vars;
    
    ExperimentSetup() = default;

    ExperimentSetup(int n_conditions, int n_dep_vars, int n_obs, std::vector<double> means, std::vector<double> vars)
    : nc(n_conditions), nd(n_dep_vars), nobs(n_obs), true_means(means), true_vars(vars) {
        ng = nc * nd;
    }
    
    // For Multivariate
    ExperimentSetup(int n_conditions, int n_dep_vars, int n_obs, std::vector<double> means, std::vector<std::vector<double>> sigma) 
    : nc(n_conditions), nd(n_dep_vars), nobs(n_obs), true_means(means), true_sigma(sigma) {        
        ng = nc * nd;
        isMultivariate = true;
    }

    // For LatentModel
    ExperimentSetup(int n_conditions, int n_dep_vars, int n_items, int n_obs,
                    std::vector<double> factor_loading, std::vector<double> factor_means, std::vector<std::vector<double>> facror_cov, std::vector<std::vector<double>> error_cov)
    : nc(n_conditions), nd(n_dep_vars), ni(n_items), nobs(n_obs), 
    factorLoadings(factor_loading),  factorMeans(factor_means), factorCov(facror_cov), errorCov(error_cov)
         {
        ng = nc * nd;
        nrows = ng * ni;
        // isMultivariate = true;      // I don't think this will be necessary!
        // FIXME: Having this crashes the code, I did something wrong in initalization
    }
    
    
    ~ExperimentSetup() = default;

    // bool _is_correalted = false;    // If true, then we are expecting a matrix.


    // Multivariate Experiments
    bool isMultivariate = false;
    double cov = 0.;
    std::vector<double> true_cov;
    std::vector<std::vector<double>> true_sigma;
    
    // Latent Experiments
    bool isFactorModel = false;
    std::vector<double> factorLoadings;                 ///< \f$\lambda\f$
    std::vector<double> factorMeans;                    ///< \f$\beta\f$
    std::vector<std::vector<double>> factorCov;         ///<
    std::vector<std::vector<double>> errorCov;          ///<
    

//private:
    
};

#endif //SAMPP_EXPERIMENTSETUP_H
