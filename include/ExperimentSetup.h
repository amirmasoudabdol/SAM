//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENTSETUP_H
#define SAMPP_EXPERIMENTSETUP_H

#include <vector>



enum ExperimentType {
    FixedModel,
    RandomModel,
    LatentModel
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
    int ni = 0;     ///< Number of items for each latent variable, if `isLatentExpr` is `true`.
    int ng;         ///< \brief Total number of groups
                    ///< Always calculated as \f$n_g = n_c * n_d\f$, unless the simulation contains latent variables, \f$n_g = n_c * n_d * n_i\f$
    
    int nobs;       ///< Number of observations in each group

    double alpha = 0.05;
//    std::vector<std::string> group_names;
//    std::vector<std::string> dv_names;

    std::vector<double> true_means;
    std::vector<double> true_vars;
    
    ExperimentSetup();

    ExperimentSetup(int n_conditions, int n_dvs, int n_obs, std::vector<double> means, std::vector<double> vars);
    ExperimentSetup(int n_conditions, int n_dvs, int n_obs, std::vector<double> means, std::vector<std::vector<double>> true_sigma);

    ~ExperimentSetup() { };

    // bool _is_correalted = false;    // If true, then we are expecting a matrix.


    // Multivariate Experiments
    bool isMultivariate = false;
    double cov = 0.;
    std::vector<double> true_cov;
    std::vector<std::vector<double>> true_sigma;
    
    // Latent Experiments
    bool isLatentExpr = false;
    std::vector<double> latentA;
    

//private:
    
};

#endif //SAMPP_EXPERIMENTSETUP_H
