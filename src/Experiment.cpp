//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include "Experiment.h"

using namespace sam;

Experiment::Experiment(json &experiment_config) {

  // Setup the Experiment
  this->setup = ExperimentSetup(experiment_config);

  // Setup the Data Strategy
  this->data_strategy = DataStrategy::build(experiment_config["data_strategy"]);

  // Setup the Test Strategy
  this->test_strategy = TestStrategy::build(experiment_config["test_strategy"]);

  // Setup Effect Strategy
  this->effect_strategy =
      EffectStrategy::build(experiment_config["effect_strategy"]);

  // Initializing the memory
  initResources();
}

Experiment::Experiment(ExperimentSetup &e) : setup{e} {

  data_strategy =
      std::shared_ptr<DataStrategy>(DataStrategy::build(setup.dsp_conf));

  test_strategy =
      std::shared_ptr<TestStrategy>(TestStrategy::build(setup.tsp_conf));

  effect_strategy =
      std::shared_ptr<EffectStrategy>(EffectStrategy::build(setup.esp_conf));

  initResources();
};

Experiment::Experiment(ExperimentSetup &e, std::shared_ptr<DataStrategy> &ds,
                       std::shared_ptr<TestStrategy> &ts,
                       std::shared_ptr<EffectStrategy> &efs)
    : setup{e}, data_strategy{ds}, test_strategy{ts}, effect_strategy{efs} {

  initResources();
};

void Experiment::generateData() { data_strategy->genData(this); }

void Experiment::runTest() { test_strategy->run(this); }

void Experiment::initExperiment() {
  initResources();
  generateData();
  calculateStatistics();
  calculateEffects();
}

void Experiment::initResources() {

  groups_.resize(setup.ng());

  for (int g{0}; g < setup.ng(); ++g) {

    if (g < setup.nd())
      groups_[g].gtype = GroupType::Control;
    else
      groups_[g].gtype = GroupType::Treatment;

    groups_[g].id_ = g;
  }
}

void Experiment::calculateStatistics() {

  for (int g{0}; g < groups_.size(); ++g)
    groups_[g].updateStats();
}

void Experiment::calculateEffects() { effect_strategy->computeEffects(this); }

void Experiment::recalculateEverything() {

  this->calculateStatistics();

  this->calculateEffects();

  this->runTest();
}

void Experiment::clear() {

  for (auto &group : groups_) {
    group.clear();
  }

  std::sort(groups_.begin(), groups_.end(),
            [](const auto &l, const auto &r) { return l.id_ < r.id_; });
}
