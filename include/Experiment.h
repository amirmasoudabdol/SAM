//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENT_H
#define SAMPP_EXPERIMENT_H

#include <vector>

#include "DataGenStrategy.h"
#include "ExperimentSetup.h"
#include "EffectEstimators.h"
#include "TestStrategy.h"
//#include "HackingStrategies.h"

class Experiment {
    
    // TODO: This is starting to look like that it needs to be an Abstract
    // There is a lot of different between different Experiments.

public:
    ExperimentSetup setup;

//    std::vector<int> nobs;
    std::vector<std::vector<double>> measurements;
    std::vector<double> means;
    std::vector<double> vars;
    std::vector<double> ses;
    std::vector<double> statistics;
    std::vector<double> pvalues;
    std::vector<double> effects;
    
    bool latentDesign = false;
    std::vector<std::vector<double> > items;
    std::vector<double> latent_means;
    std::vector<double> latent_variances;
    std::vector<std::vector<double>> latent_cov_matrix;

    // Need a copy constructor
    // Need a move constructor
//    Experiment();
    ~Experiment() {
        // TODO: Use shared_ptr<> to make sure that I don't have
        // to do cleanup
    };

    Experiment(ExperimentSetup& e) : setup(e) {
        // I can do things here, like some of the r/**/esizing
        // nobs.resize(e.ng)
//        testStrategy = nullptr;
//        dataStrategy = nullptr;
    };
//    Experiment(ExperimentSetup, DataGenStrategy);
//    Experiment(ExperimentSetup setup) : { experiment_setup(setup) }

    TestStrategy* testStrategy;
    void setTestStrategy(TestStrategy* t);
    // I'm not even sure if this is necessary or not, I need to run some tests
    void pointTestStrategyToThis();
    void runTest();

    DataGenStrategy* dataStrategy;
    void setDataStrategy(DataGenStrategy* d);

    // Initialize the Experiment/**/
    void allocateResources();
    void initExperiment();
    void generateData();
    void calculateStatistics();
    void calculateEffects();


    // Hacking Meta-data
    bool isHacked = false;     ///< Indicates if any hacking routine has
                               ///< been applied on the experiment
//    std::vector<HackingStrategies> hackingHistory;
    
    // Randomize
    // void randomizeData();

    // std::vector<std::vector<double>> setMeasurements();
    // std::vector<std::vector<double>> getMeasurements();

     // void setDataStrategy(std::string strategy);
    // void genData();
    // void genDataFor(int gid, int dvid, int n);

    // I actually don't need to write a copy instructor,
    // because I don't mind if pointers are pointing to the old testStrategy,
    // and dataStrategy, that's fine.
    // TODO: However, I need to make sure that the testStrategy points to the new
    // copy of the Experiment. The `updateExperimentPointer()` and `pointTestStrategyToThis()`
    // will take care of this, but maybe I can have them inside the copy and = to
    // comply with RAII.
    
//    Experiment(const Experiment& other){
//
//    }
//    
//    Experiment& operator=(const Experiment& lhs){
//
//    };

    
private:




};

#endif //SAMPP_EXPERIMENT_H
