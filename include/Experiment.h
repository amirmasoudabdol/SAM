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
//    Experiment(ExperimentSetup setup) : { experiment_setup(setup) }
    Experiment();
//    Experiment(ExperimentSetup);
//    Experiment(ExperimentSetup, DataGenStrategy);
    ~Experiment();

    // Experiment Setup
    ExperimentSetup experiment_setup;

    // Initialize the Experiment
    void initData();
    void calculateStatistics();

    // Randomize
    void randomizeData();

    std::vector<std::vector<double>> setMeasurements();
    std::vector<std::vector<double>> getMeasurements();

    void setDataStrategy(DataGenStrategy);
    void genData();
    void genNDataFor(int gid, int dvid, int n);

private:

    DataGenStrategy _data_strategy;

    std::vector<std::vector<double>> _measurements;
    std::vector<double> _means;
    std::vector<double> _vars;
    std::vector<double> _ses;
    std::vector<double> _statistics;
    std::vector<double> _pvalues;
    std::vector<double> _effects;

};

#endif //SAMPP_EXPERIMENT_H
