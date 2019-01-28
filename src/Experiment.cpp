//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include "Experiment.h"
#include <iostream>
#include <string>
#include <numeric>
#include <algorithm>
#include "Utilities.h"
#include "gsl/gsl_statistics.h"


void Experiment::sayHi() {
    std::cout << "Experiment says Hi";
}

void Experiment::generateData() {
    measurements = dataStrategy->genData();
}

void Experiment::setDataStrategy(std::string strategy) {
//    if (strategy == "Fixed Model"){
//        dataStrategy = FixedEffectStrategy(setup);
//    }
}

void Experiment::setDataStrategy(DataGenStrategy *strategy) {
    dataStrategy = strategy;
}

void Experiment::initExperiment() {
    allocateResources();
    generateData();
    calculateStatistics();
}

void Experiment::allocateResources() {
    // std::cout << "alloc, ng is :" << setup.ng;
    // TODO: There is an issue with the allocation;
    // TODO: I either need to initiate or push_back
//    means.resize(setup.ng, 0);
//    vars.resize(setup.ng, 0);
//    ses.resize(setup.ng, 0);
    // statistics.resize(setup.ng, 0);
    // pvalues.resize(setup.ng, 0);
    // effects.resize(setup.ng, 0);
}

void Experiment::calculateStatistics() {
    std::transform(measurements.begin(), measurements.end(), std::back_inserter(means), mean);
    for (int i = 0; i < setup.ng; ++i) {
        vars.push_back(gsl_stats_variance_m(measurements[i].data(), 1, measurements[i].size(), means[i]));
        ses.push_back(sqrt(vars[i] / measurements[i].size()));
    }


   // std::cout << means[0] << ", ";
}
