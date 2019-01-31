//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include "Experiment.h"
#include "TestStrategy.h"

#include <iostream>
#include <string>
#include <numeric>
#include <algorithm>
#include "Utilities.h"
#include "gsl/gsl_statistics.h"


void Experiment::sayHi() {
    std::cout << "Experiment says Hi!\n";
}

void Experiment::generateData() {
    measurements = dataStrategy->genData();
    // std::cout << measurements[0][0] << "<\n";
}

void Experiment::setDataStrategy(DataGenStrategy* d) {
    dataStrategy = d;
}

void Experiment::setTestStrategy(TestStrategy *t){
    testStrategy = t;
}

void Experiment::runTest(){
    testStrategy->run();
}

void Experiment::initExperiment() {
    allocateResources();
    generateData();
    calculateStatistics();
    calculateEffects();
}

void Experiment::allocateResources() {
    // std::cout << "alloc, ng is :" << setup.ng;
    // TODO: There is an issue with the allocation;
    // TODO: I either need to initiate or push_back
    means.resize(setup.ng, 0);
    vars.resize(setup.ng, 0);
    ses.resize(setup.ng, 0);
    statistics.resize(setup.ng, 0);
    pvalues.resize(setup.ng, 0);
    effects.resize(setup.ng, 0);
}

void Experiment::calculateStatistics() {
    // TODO: This needs to be change! I need to do the init/de-init better.
    means.clear();
    std::transform(measurements.begin(), measurements.end(), std::back_inserter(means), mean);
    for (int i = 0; i < setup.ng; ++i) {
        vars[i] = gsl_stats_variance_m(measurements[i].data(), 1, measurements[i].size(), means[i]);
        ses[i] = sqrt(vars[i] / measurements[i].size());
    }
    // std::cout << vars[0] << ": v\n";


   // std::cout << means[0] << ", ";
}

void Experiment::calculateEffects() {
    effects = means;
}


// This is a tricky business!
void Experiment::pointTestStrategyToThis() {
    testStrategy->updateExperimentPointer(this);
}
