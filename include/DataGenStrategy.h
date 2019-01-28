//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_DATAGENSTRATEGY_H
#define SAMPP_DATAGENSTRATEGY_H

#include <vector>

#include "ExperimentSetup.h"
#include "RandomNumberGenerator.h"

class DataGenStrategy{

public:
    virtual std::vector<std::vector<double>> genData() = 0;
    
};

class FixedEffectStrategy : public DataGenStrategy {

public:

    ExperimentSetup setup;
    RandomNumberGenerator rngEngine;


    FixedEffectStrategy(ExperimentSetup& e, RandomNumberGenerator& rng_engine) :
        setup(e), rngEngine(rng_engine)
    {
        
    };

    std::vector<std::vector<double>> genData();


private:

};

#endif //SAMPP_DATAGENSTRATEGY_H
