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

///
/// This constructs an Experiment, and allocates the resources necessary for the
/// experiment to setup its dependent variables, etc. Using the config parameters of the
/// Data, Test, and Effect strategies, it also initializes those to be used by the
/// Researcher or the Experiment itself
///
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

///
/// Similar to the other constructor, it constructs all necessary internals of the
/// experiment, however, it uses the ExperimentSetup and its specification as the
/// source of the configurations.
///
Experiment::Experiment(ExperimentSetup &e) : setup{e} {

  data_strategy =
      std::shared_ptr<DataStrategy>(DataStrategy::build(setup.dsp_conf));

  test_strategy =
      std::shared_ptr<TestStrategy>(TestStrategy::build(setup.tsp_conf));

  effect_strategy =
      std::shared_ptr<EffectStrategy>(EffectStrategy::build(setup.esp_conf));

  initResources();
}

/// This directly constructs the expreiment by directly constructing its internal based on
/// the set of given parameters.
///
/// @note This is mainly used by the ExperimentBuilder
///
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

///
/// It allocates the necessary memeory for the dependent variables.
///
/// @todo This should be done nicer! It's only being done like this at the moment
/// because I need to update each dv's id.
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

void Experiment::clear() {

  for (auto &dv : dvs_) {
    dv.clear();
  }

  // This is strange, and I don't want it! It's here because Policy might mess up the
  // order of DVs when it runs some of the function on a group of them. I think if I use
  // std::reference_wrapper I can avoid this but that needs some extra work.
  std::sort(dvs_.begin(), dvs_.end(),
            [](const auto &l, const auto &r) { return l.id_ < r.id_; });
}

/// Adds new candidates to the list of selected candidates
void Experiment::addNewCandidates(const std::vector<Submission>& subs) {
  candidates.value().insert(candidates.value().end(), subs.begin(), subs.end());
}


/// It sets the overall hacked status of the experiment to true. This has the power to
/// overwrites the hacked status of the dependent variables.
void Experiment::setHackedStatus(const bool status) {
  is_hacked = status;
}

///
void Experiment::setHackedStatusOf(const std::vector<size_t> &idxs, const bool status) {
  std::for_each(idxs.begin(), idxs.end(),
                [this, &status](auto &i){
                    this->dvs_[i].setHackedStatus(status);
                  });
}


void Experiment::setCandidateStatusOf(const std::vector<size_t> &idxs, const bool status) {
  std::for_each(idxs.begin(), idxs.end(),
                [this, &status](auto &i){
                    this->dvs_[i].setCandidateStatus(status);
                  });
}

/// An experiment is hacked if its hacked status has been set to `true`, or one of its
/// dependent variables has been flagged as hacked
bool Experiment::isHacked() const {
  return is_hacked or
         std::any_of(dvs_.begin(), dvs_.end(), [](auto &dv){return dv.isHacked();});
};

bool Experiment::isModified() const {
  return std::any_of(dvs_.begin(), dvs_.end(), [](auto &dv){return dv.isModified();});
};

bool Experiment::hasCandidates() const {
  return std::any_of(dvs_.begin(), dvs_.end(), [](auto &dv){return dv.isCandidate();});
}
