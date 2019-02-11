//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include "ExperimentSetup.h"

std::ostream& operator<<(std::ostream& os, ExperimentType et)
{
    switch(et)
    {
        case FixedModel   : os << "Fixed Model";    break;
        case RandomModel      : os << "Random Model"; break;
        case FactorModel     : os << "Factor Model";  break;
        default    : os.setstate(std::ios_base::failbit);
    }
    return os;
}

//ExperimentSetup::ExperimentSetup(int n_conditions, int n_dvs, int n_obs, std::vector<double> means, std::vector<double> vars)
//        : nc(n_conditions), nd(n_dvs), nobs(n_obs), true_means(means), true_vars(vars) {
//    ng = nc * nd;
//}
