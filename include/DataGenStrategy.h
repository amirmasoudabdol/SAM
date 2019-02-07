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


    FixedEffectStrategy(ExperimentSetup& es, RandomNumberGenerator& rng_engine) :
        setup(es), rngEngine(rng_engine){
        // TODO: I might need to pass rng_engine as a pointer too, 
        // because I might ran into the same problem that it loses its state
        // and produce unreliable results.
    };

    std::vector<std::vector<double>> genData();
    std::vector<std::vector<double>> genNewObservationsForAllGroups(int n_new_obs);
    std::vector<double> genNewObservationsFor(int g, int n_new_obs);


private:

};

class LatentDataStrategy : public DataGenStrategy {
public:
    ExperimentSetup setup;
    RandomNumberGenerator rngEngine;
    

    LatentDataStrategy(ExperimentSetup& es, RandomNumberGenerator& rng_engine) :
    setup(es), rngEngine(rng_engine) {
        
    };
    
    std::vector<std::vector<double>> genData();
    std::vector<std::vector<double>> genNewObservationsForAllGroups(int n_new_obs);
    std::vector<double> genNewObservationsFor(int g, int n_new_obs);
    

//    std::vector<double> computeItemsMean();     // This will fill the experiment->measurements
//    std::vector<double> _compute_latent_means();
//    std::vector<double> _compute_latent_variances();
//    std::vector<std::vector<double> > _construct_latent_cov_matrix();
    
    void latentModelTest();
    
    // Routine
    //  1. Compute Latent Means
    //  2. Compute Latent Variances
    //  3. Construct Latent Covarinace Matrix
    //  4. Generate data with _latent_means, _latent_variances;
    
    
//private:
//    std::vector<double> fMeans;
//    std::vector<double> fVars;
//    std::vector<std::vector<double>> fSigma;
//
//    std::vector<double> _a;     ///< 1 - setup.latent_alpha;
//    std::vector<double> epsilonMeans;
//    std::vector<std::vector<double>> epsilonSigma;
    
    
    
};

#endif //SAMPP_DATAGENSTRATEGY_H
