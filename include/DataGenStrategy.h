//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_DATAGENSTRATEGY_H
#define SAMPP_DATAGENSTRATEGY_H

#include <vector>

#include "ExperimentSetup.h"
#include "RandomNumberGenerator.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class Experiment;

/**
 \brief Abstract class for Data Strategies

 A DataGenStrategy should at least two methods, `genData` and `genNewObservationForAllGroups`.
 The former is mainly used to populate a new Experiment while the latter is being used by some 
 hacking strategies, e.g. OptionalStopping, where new data — from the same population — is needed.

 \note Each Data Strategy should have access to an instance of RandomNumberGenerator. This is usually done
 by creating a desired _random engine_ and passing the pointer to the DataGenStrategy.
 */
class DataGenStrategy {

public:

    static DataGenStrategy* buildDataStrategy(ExperimentSetup& setup);
    
    virtual void genData(Experiment* experiment) = 0;
    virtual std::vector<std::vector<double>> genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs) = 0;
    virtual std::vector<double> genNewObservationsFor(Experiment* experiment, int g, int n_new_obs) = 0;
    
    
};

/**
 The fixed-effect data strategy will produce data from a fixed-effect
 model with the given \f$\mu\f$ and \f$sigma\f$.
 */
class FixedEffectStrategy : public DataGenStrategy {

public:
    
    FixedEffectStrategy(ExperimentSetup& setup) {
        _main_seed = rand();
        _sec_seed = rand();
        
        mainRngStream = new RandomNumberGenerator(_main_seed, setup.isCorrelated);
    };
    
    void genData(Experiment* experiment);
    std::vector<std::vector<double>> genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs);
    std::vector<double> genNewObservationsFor(Experiment* experiment, int g, int n_new_obs);
    
private:
    int _main_seed;
    int _sec_seed;
    RandomNumberGenerator *mainRngStream;
};

/**
 A Data Strategy for constructing a general [Structural Equaiton Model](https://en.wikipedia.org/wiki/Structural_equation_modeling).

 \note LatentDataStrategy will generate individual items, therefore it might be slower than other models.
 */
class LatentDataStrategy : public DataGenStrategy {

public:
    
    LatentDataStrategy(ExperimentSetup& setup){
        _main_seed = rand();
        _sec_seed = rand();
        
        mainRngStream = new RandomNumberGenerator(_main_seed, setup.isCorrelated);
    }
    
    void genData(Experiment* experiment);
    std::vector<std::vector<double>> genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs);
    std::vector<double> genNewObservationsFor(Experiment* experiment, int g, int n_new_obs);
    
private:
    int _main_seed;
    int _sec_seed;
    RandomNumberGenerator *mainRngStream;
    
};

#endif //SAMPP_DATAGENSTRATEGY_H
