//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_DATAGENSTRATEGY_H
#define SAMPP_DATAGENSTRATEGY_H

#include <vector>

#include "ExperimentSetup.h"
#include "RandomNumberGenerator.h"

class DataGenStrategy {

public:
    virtual std::vector<std::vector<double>> genData() = 0;
    virtual std::vector<std::vector<double>> genNewObservationsForAllGroups(int n_new_obs) = 0;
    virtual std::vector<double> genNewObservationsFor(int g, int n_new_obs) = 0;
    
};

class FixedEffectStrategy : public DataGenStrategy {

public:

    ExperimentSetup setup;
    RandomNumberGenerator rngEngine;


    FixedEffectStrategy(ExperimentSetup& e, RandomNumberGenerator& rng_engine) :
        setup(e), rngEngine(rng_engine){
        // TODO: I might need to pass rng_engine as a pointer too, 
        // because I might ran into the same problem that it loses its state
        // and produce unreliable results.
    };

    std::vector<std::vector<double>> genData();
    std::vector<std::vector<double>> genNewObservationsForAllGroups(int n_new_obs);
    std::vector<double> genNewObservationsFor(int g, int n_new_obs);


private:

};

#endif //SAMPP_DATAGENSTRATEGY_H
