//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_DATAGENSTRATEGY_H
#define SAMPP_DATAGENSTRATEGY_H

#include <vector>
#include <armadillo>

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
class DataStrategy {

public:

    static DataStrategy* build(ExperimentSetup &setup);
    
    virtual ~DataStrategy() = 0;
    
    virtual void genData(Experiment* experiment) = 0;
    virtual std::vector<arma::Row<double> > genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs) = 0;
    virtual arma::Row<double> genNewObservationsFor(Experiment* experiment, int g, int n_new_obs) = 0;
    
    
};

/**
 The fixed-effect data strategy will produce data from a fixed-effect
 model with the given \mu and sigma.
 */
class LinearModelStrategy : public DataStrategy {

public:
    
    LinearModelStrategy(ExperimentSetup& setup) {
        _main_seed = rand();
        _sec_seed = rand();
        
        mainRngStream = new RandomNumberGenerator(_main_seed);
        secRngStream = new RandomNumberGenerator(_sec_seed);
    };
    
    void genData(Experiment* experiment);
    std::vector<arma::Row<double> > genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs);
    arma::Row<double> genNewObservationsFor(Experiment* experiment, int g, int n_new_obs);
    
private:
    int _main_seed;
    int _sec_seed;
    RandomNumberGenerator *mainRngStream;
    RandomNumberGenerator *secRngStream;
};

/**
 A Data Strategy for constructing a general [Structural Equaiton Model](https://en.wikipedia.org/wiki/Structural_equation_modeling).

 \note LatentDataStrategy will generate individual items, therefore it might be slower than other models.
 */
class LatentDataStrategy : public DataStrategy {

public:
    
    LatentDataStrategy(ExperimentSetup& setup){
        _main_seed = rand();
        _sec_seed = rand();
        
        mainRngStream = new RandomNumberGenerator(_main_seed);
        secRngStream = new RandomNumberGenerator(_sec_seed);
    }
    
    void genData(Experiment* experiment);
    std::vector<arma::Row<double> > genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs);
    arma::Row<double> genNewObservationsFor(Experiment* experiment, int g, int n_new_obs);
    
private:
    int _main_seed;
    int _sec_seed;
    RandomNumberGenerator *mainRngStream;
    RandomNumberGenerator *secRngStream;
    
};

#endif //SAMPP_DATAGENSTRATEGY_H