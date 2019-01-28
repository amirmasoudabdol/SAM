//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include "ExperimentSetup.h"

ExperimentSetup::ExperimentSetup(int n_conditions, int n_dvs, int n_obs, std::vector<double> means, std::vector<double> vars)
        : nc(n_conditions), nd(n_dvs), nobs(n_obs), true_means(means), true_vars(vars) {
    ng = nc * nd;
}
