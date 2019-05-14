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
    data_strategy->genData(this);
}

void Experiment::runTest(){
    test_strategy->run(this);
}

void Experiment::initExperiment() {
    initResources(setup.ng());
    generateData();
    calculateStatistics();
    calculateEffects();
}

// TODO: Still not happy with this!
void Experiment::initResources(int len) {

    means.zeros(len);
    vars.zeros(len);
    ses.zeros(len);
    statistics.zeros(len);
    pvalues.zeros(len);
    
    for (auto &estimator : effect_size_estimators) {
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
    
}

void Experiment::calculateEffects() {

    for (auto &estimator : effect_size_estimators){
        estimator->computeEffects(this);
    }

}


void Experiment::recalculateEverything() {

    this->calculateStatistics();
    
    this->calculateEffects();
        
    this->runTest();
    
}


Experiment::Experiment(json &experiment_config) { 
    
    // Setup the Experiment
    this->setup = ExperimentSetup(experiment_config["ExperimentParameters"]);

    // Setup the Data Strategy
    this->data_strategy = DataStrategy::build(experiment_config["ExperimentParameters"]["data-strategy"]);
    
    // Setup the Test Strategy
    this->test_strategy = TestStrategy::build(experiment_config["ExperimentParameters"]["test-strategy"]);
    
    for (auto &estimator : experiment_config["ExperimentParameters"]["effect-estimators"]){
        this->effect_size_estimators.push_back(EffectSizeEstimator::build(estimator));
    }
    
    // Initializing the memory
    initResources(setup.ng());
    
}


void Experiment::randomize() {
    if (setup.is_n_randomized) {
        setup.randomize_nObs();
    }
}
