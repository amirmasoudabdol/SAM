//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include "Experiment.h"
#include "TestStrategy.h"

#include <iostream>
#include <string>
#include <numeric>
#include <algorithm>
#include <armadillo>
#include "Utilities.h"
#include "gsl/gsl_statistics.h"

void Experiment::generateData() {
    dataStrategy->genData(this);
}

void Experiment::setDataStrategy(DataGenStrategy* d) {
    dataStrategy = d;
}

void Experiment::setTestStrategy(TestStrategy *t){
    testStrategy = t;
}

void Experiment::runTest(){
    testStrategy->run(this);
}

void Experiment::initExperiment() {
    initResources();
    generateData();
    calculateStatistics();
    calculateEffects();
}

void Experiment::initResources() {
    // std::cout << "alloc, ng is :" << setup.ng;
    // TODO: There is an issue with the allocation;
    // TODO: I either need to initiate or push_back
    // FIXME: This is very error prune, since it's fixed number
    means.zeros(setup.ng);
    vars.zeros(setup.ng);
    ses.zeros(setup.ng);
    statistics.zeros(setup.ng);
    pvalues.zeros(setup.ng);
    effects.zeros(setup.ng);
}

void Experiment::calculateStatistics() {
    // TODO: This needs to be change! I need to do the init/de-init better.
//    means.clear();
    // std::transform(measurements.begin(), measurements.end(), std::back_inserter(means), mean);
    // for (auto &v : measurements){
    //     means
    // }

    for (int i = 0; i < setup.ng; ++i) {
        means[i] = mean(measurements[i]);
//        vars[i] = gsl_stats_variance_m(measurements[i].data(), 1, measurements[i].size(), means[i]);
        vars[i] = var(measurements[i]);
        ses[i] = sqrt(vars[i] / measurements[i].size());
    }
    
    
}

void Experiment::calculateEffects() {
    effects = means;
}


void Experiment::recalculateEverything() {

    experiment->calculateStatistics();
    
    experiment->calculateEffects();
        
    experiment->runTest();
    
}


Experiment::Experiment(json &config) { 
    // TODO: This should initialize everything, also set TestStrategy, ...
    this->setup = ExperimentSetup(config["Experiment Parameters"]);

    this->dataStrategy = DataGenStrategy::buildDataStrategy(setup);
    
    this->testStrategy = TestStrategy::buildTestStrategy(setup);
    
}


void Experiment::randomize() {
    if (setup.isNRandomized) {
        setup.randomize_nObs();
    }
}
