//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <DataGenStrategy.h>

std::vector<std::vector<double>> FixedEffectStrategy::genData()  {
    return this->rngEngine.normal(setup.true_means, setup.true_vars, setup.nobs);
}

std::vector<std::vector<double>> FixedEffectStrategy::genNewObservationsForAllGroups(int n_new_obs) {
    return this->rngEngine.normal(setup.true_means, setup.true_vars, n_new_obs);
}

std::vector<double> FixedEffectStrategy::genNewObservationsFor(int g, int n_new_obs) {

    return std::vector<double>();
}
