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

using namespace sam;

void Experiment::generateData() {
    dataStrategy->genData(this);
}

void Experiment::setDataStrategy(DataStrategy* d) {
    dataStrategy = d;
}

void Experiment::setTestStrategy(TestStrategy *t){
    testStrategy = t;
}

void Experiment::runTest(){
    testStrategy->run(this);
}

void Experiment::initExperiment() {
    initResources(setup.ng);
    generateData();
    calculateStatistics();
    calculateEffects();
}

void Experiment::initResources(int len) {
    // TODO: There is an issue with the allocation;
    // TODO: I either need to initiate or push_back
    // FIXME: This is very error prune, since it's fixed number
    means.zeros(len);
    vars.zeros(len);
    ses.zeros(len);
    statistics.zeros(len);
    pvalues.zeros(len);
//    effects.zeros(len);
    
    for (auto &estimator : effectSizeEstimators) {
        effects[estimator->name].zeros(len);
    }
    
    sigs.zeros(len);
}

void Experiment::calculateStatistics() {
    
    for (int i = 0; i < measurements.size(); ++i) {
        means[i] = arma::mean(measurements[i]);
        vars[i] = arma::var(measurements[i]);
        ses[i] = sqrt(vars[i] / measurements[i].size());
    }
    
//    ses = sqrt(vars / nobs);
    
}

void Experiment::calculateEffects() {
//    effects = means;
    for (auto &estimator : effectSizeEstimators){
        estimator->computeEffects(this);
    }
//    effectSizeEstimator->computeEffects(this);
}


void Experiment::recalculateEverything() {

    this->calculateStatistics();
    
    this->calculateEffects();
        
    this->runTest();
    
}


Experiment::Experiment(json &config) { 
    // TODO: This should initialize everything, also set TestStrategy, ...
    this->setup = ExperimentSetup(config["ExperimentParameters"]);

    this->dataStrategy = DataStrategy::build(setup);
    
    this->testStrategy = TestStrategy::build(config["ExperimentParameters"]["test-strategy"]);
    
    for (auto &estimator : config["ExperimentParameters"]["effect-estimators"]){
        this->effectSizeEstimators.push_back(EffectSizeEstimator::build(estimator));
    }
    
}


void Experiment::randomize() {
    if (setup.is_n_randomized) {
        setup.randomize_nObs();
    }
}
