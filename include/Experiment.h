//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENT_H
#define SAMPP_EXPERIMENT_H

#include <vector>

#include "DataGenStrategy.h"
#include "ExperimentSetup.h"
#include "EffectEstimators.h"

class Experiment {


public:
    ExperimentSetup setup;
    
    // ExperimentSetup setup;

    std::vector<int> nobs;
    std::vector<std::vector<double>> measurements;
    std::vector<double> means;
    std::vector<double> vars;
    std::vector<double> ses;
    std::vector<double> statistics;
    std::vector<double> pvalues;
    std::vector<double> effects;


    // Need a copy constructor
    // Need a move constructor
    Experiment();
    ~Experiment() {};

    Experiment(ExperimentSetup& e) : setup(e) {
        // I can do things here, like some of the resizing
        // nobs.resize(e.ng)
    };
//    Experiment(ExperimentSetup, DataGenStrategy);
//    Experiment(ExperimentSetup setup) : { experiment_setup(setup) }


     DataGenStrategy* dataStrategy;

    // Initialize the Experiment
     void allocateResources();
     void initExperiment();
     void generateData();
     void calculateStatistics();

    // Randomize
    // void randomizeData();

    // std::vector<std::vector<double>> setMeasurements();
    // std::vector<std::vector<double>> getMeasurements();

     void setDataStrategy(DataGenStrategy *strategy);
     void setDataStrategy(std::string strategy);
    // void genData();
    // void genDataFor(int gid, int dvid, int n);


    void sayHi();

private:




};

#endif //SAMPP_EXPERIMENT_H
