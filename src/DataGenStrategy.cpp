//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include "DataGenStrategy.h"

std::vector<std::vector<double>> FixedEffectStrategy::genData()  {

    return this->rngEngine.normal(setup.true_means, setup.true_vars, setup.nobs);
}
