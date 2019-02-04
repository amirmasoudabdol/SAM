//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENTSETUP_H
#define SAMPP_EXPERIMENTSETUP_H

#include <vector>



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
    
    ExperimentSetup(int n_conditions, int n_dvs, int n_obs, std::vector<double> means, std::vector<std::vector<double>> sigma);

    ~ExperimentSetup() = default;

    // bool _is_correalted = false;    // If true, then we are expecting a matrix.


    // Multivariate Experiments
    bool isMultivariate = false;
    double cov = 0.;
    std::vector<double> true_cov;
    std::vector<std::vector<double>> true_sigma;
    
    // Latent Experiments
    bool isFactorModel = false;
    std::vector<double> factorLoads;
    

//private:
    
};

#endif //SAMPP_EXPERIMENTSETUP_H
