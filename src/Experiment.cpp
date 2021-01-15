//===-- Experiment.cpp - Experiment Implementation ------------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-01-22.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the implementation of the Experiment, and
/// the ExperimentBuilder classes. Both
///
//===----------------------------------------------------------------------===//

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
}

Experiment::Experiment(ExperimentSetup &e, std::shared_ptr<DataStrategy> &ds,
                       std::shared_ptr<TestStrategy> &ts,
                       std::shared_ptr<EffectStrategy> &efs)
    : setup{e}, data_strategy{ds}, test_strategy{ts}, effect_strategy{efs} {

  initResources();
}

void Experiment::generateData() { data_strategy->genData(this); }

void Experiment::runTest() { test_strategy->run(this); }

void Experiment::initExperiment() {
  clear();
  initResources();
}

void Experiment::initResources() {

  dvs_.resize(setup.ng());

  for (int g{0}; g < setup.ng(); ++g) {
    dvs_[g].id_ = g;
  }
}

void Experiment::calculateStatistics() {

  for (int g{0}; g < dvs_.size(); ++g)
    dvs_[g].updateStats();
}

void Experiment::calculateEffects() { effect_strategy->computeEffects(this); }

void Experiment::recalculateEverything() {

  this->calculateStatistics();

  this->calculateEffects();

  this->runTest();
}

void Experiment::recalculateEverythingForGroup(size_t inx) {

  this->dvs_[inx].updateStats();

  //  this->calculateEffects();
  //
  //  this->runTest();
}

void Experiment::clear() {

  for (auto &group : dvs_) {
    group.clear();
  }

  std::sort(dvs_.begin(), dvs_.end(),
            [](const auto &l, const auto &r) { return l.id_ < r.id_; });
}

void Experiment::updateCandidatesList(const std::vector<Submission>& subs) {
  candidates.value().insert(candidates.value().end(), subs.begin(), subs.end());
}


void Experiment::setHackedStatus(const bool status) {
  is_hacked = status;
  std::for_each(dvs_.begin(), dvs_.end(),
                [&status](auto &g){
    g.is_hacked_ = status;
  });
}

void Experiment::setHackedStatusOf(const std::vector<size_t> &idxs, const bool status) {
  std::for_each(idxs.begin(), idxs.end(),
                [this, &status](auto &i){
                    this->dvs_[i].is_hacked_ = status;
                  });
}


void Experiment::setCandidateStatusOf(const std::vector<size_t> &idxs, const bool status) {
  std::for_each(idxs.begin(), idxs.end(),
                [this, &status](auto &i){
                    this->dvs_[i].is_candidate_ = status;
                  });
}
