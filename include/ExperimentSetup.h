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


class ExperimentSetup {


public:
    
    ExperimentType experimentType;
    
    std::vector<double> latent_alpha;

    int ni;     ///< Number of items in each DV
    
    int nc = 1;
    int nd = 3;
    int ng;
    int nobs;

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


    bool isMultivariate = false;
    double cov = 0.;
    std::vector<double> true_cov;
    std::vector<std::vector<double>> true_sigma;

//private:
    
};

#endif //SAMPP_EXPERIMENTSETUP_H
