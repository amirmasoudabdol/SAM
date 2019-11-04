//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include "Experiment.h"
#include "TestStrategy.h"

#include <iostream>
#include <string>
#include <numeric>
#include <algorithm>


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

    nobs.resize(len);
    means.resize(len);
    vars.resize(len);
    stddev.resize(len);
    ses.resize(len);
    statistics.resize(len);
    pvalues.resize(len);
    effects.resize(len);
    sigs.resize(len);
    
    // BUG: This conflict with GroupPooling!
    measurements.resize(len);
}

void Experiment::calculateStatistics() {
    
    // TODO: This can be replaced by std::algorithm
    for (int i = 0; i < measurements.size(); ++i) {
        nobs[i] = measurements[i].size();
        means[i] = arma::mean(measurements[i]);
        vars[i] = arma::var(measurements[i]);
        stddev[i] = arma::stddev(measurements[i]);
        ses[i] = sqrt(vars[i] / measurements[i].size());
    }
    
}

void Experiment::calculateEffects() {
    
    effect_strategy->computeEffects(this);

}


void Experiment::recalculateEverything() {

    this->calculateStatistics();
    
    this->calculateEffects();
        
    this->runTest();
    
}


Experiment::Experiment(json &experiment_config) { 
    
    // Setup the Experiment
    this->setup = ExperimentSetup(experiment_config);

    // Setup the Data Strategy
    // this->data_strategy = DataStrategy::build(experiment_config["data_strategy"]);
    // TODO: Hey! I'm changing this to let mvnorm_dist have access to means and covs...
    this->data_strategy = DataStrategy::build(this->setup);
    
    // Setup the Test Strategy
    this->test_strategy = TestStrategy::build(experiment_config["test_strategy"]);
    
    // Setup Effect Strategy
    this->effect_strategy = EffectStrategy::build(experiment_config["effect_estimators"]);
    
    // Initializing the memory
    initResources(setup.ng());
    
}


void Experiment::randomize() {
    setup.randomize_parameters();
}
