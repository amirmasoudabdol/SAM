//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENT_H
#define SAMPP_EXPERIMENT_H

#include <vector>

#include "DataGenStrategy.h"
#include "ExperimentSetup.h"

class Experiment {
public:
    
    ExperimentSetup params;

    std::vector<std::vector<double>> measurements;
    std::vector<double> means;
    std::vector<double> vars;
    std::vector<double> ses;
    std::vector<double> statistics;
    std::vector<double> pvalues;
    std::vector<double> effects;


    Experiment();
    ~Experiment();
//    Experiment(ExperimentSetup);
//    Experiment(ExperimentSetup, DataGenStrategy);
//    Experiment(ExperimentSetup setup) : { experiment_setup(setup) }

    // Experiment Setup
    DataGenStrategy data_strategy;

    // Initialize the Experiment
    void initData();
    void calculateStatistics();

    // Randomize
    void randomizeData();

    std::vector<std::vector<double>> setMeasurements();
    std::vector<std::vector<double>> getMeasurements();

    void setDataStrategy(DataGenStrategy);
    void genData();
    void genDataFor(int gid, int dvid, int n);

private:




};

#endif //SAMPP_EXPERIMENT_H
