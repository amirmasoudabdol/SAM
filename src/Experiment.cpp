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


Experiment::Experiment(json &experiment_config) {
    
    // Setup the Experiment
    this->setup = ExperimentSetup(experiment_config);

    // Setup the Data Strategy
    this->data_strategy = DataStrategy::build(experiment_config["data_strategy"]);
    
    // Setup the Test Strategy
    this->test_strategy = TestStrategy::build(experiment_config["test_strategy"]);
    
    // Setup Effect Strategy
    this->effect_strategy = EffectStrategy::build(experiment_config["effect_strategy"]);
    
    // Initializing the memory
    initResources(setup.ng());
    
    
    
}

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
    
    // GroupData
    groups_.resize(len);
    for (int g {0}; g < len; ++g) {
        groups_[g].id_ = g;
    }
    
    
}

void Experiment::calculateStatistics() {
    
    for (int g{0}; g < groups_.size(); ++g)
        groups_[g].updateStats();
    
}

void Experiment::calculateEffects() {
    
    effect_strategy->computeEffects(this);

}


void Experiment::recalculateEverything() {

    this->calculateStatistics();
    
    this->calculateEffects();
        
    this->runTest();
    
}





void Experiment::randomize() {
    setup.randomize_parameters();
    
    // Increasing the experiment id
    expr_uuid++;
    
    /// TODO: This can have a better implementation
    /// For instance, I can have a function, like `get_id`, that yeild a UUID and everytime I
    /// ask for an ID here. This will be more robust as I can handle parallelization better.
}
