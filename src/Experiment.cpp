//===-- Experiment.cpp - Experiment Implementation -------------------------===//
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
  
  // Preparing the covariant info
  n_covariants = experiment_config["n_covariants"];

  // Initializing the memory
  initResources();
}

///
/// Since ExperimentSetup already contains the definition of Data, Test, and Effect
/// strategies, this method accepts the ExperimentSetup as it is, and only initialize
/// those strategies.
///
/// @note This can be used in cases where the underlying strategies should be
/// preserved while some experiment parameters needs to be modified.
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

///
/// This directly constructs the experiment by directly constructing its internal based on
/// the set of given parameters.
///
/// @note This is mainly used by the ExperimentBuilder
///
Experiment::Experiment(ExperimentSetup &e, std::shared_ptr<DataStrategy> &ds,
                       std::shared_ptr<TestStrategy> &ts,
                       std::shared_ptr<EffectStrategy> &es)
    : setup{e}, data_strategy{ds}, test_strategy{ts}, effect_strategy{es} {

  initResources();
}


///
/// Adds new candidates to the list of selected candidates
///
void Experiment::addNewCandidates(const std::vector<Submission>& subs) {
  candidates.value().insert(candidates.value().end(), subs.begin(), subs.end());
}

// Calculation Methods
// ------------------

void Experiment::generateData() {
  data_strategy->genData(this);
}


void Experiment::generateCovariants() {
  
  if (!is_covariants_generated) {
  
    /// @Todo Check if all groups are the same size
    covariants.resize(setup.nobs().max(), n_covariants);
    covariants.fill(0);
    
    for (int i = 0; i < n_covariants; ++i) {
      covariants.col(i).head(dvs_[i].nobs_ / 2).fill(1);
      covariants.col(i) = arma::shuffle(covariants.col(i));
    }
    
    is_covariants_generated = true;
  }
}

void Experiment::calculateTests() {
  test_strategy->run(this);
}


void Experiment::calculateStatistics() {
  
  std::for_each(dvs_.begin(), dvs_.end(), [](auto &dv){
    dv.updateStats();
  });
  
}

void Experiment::calculateEffects() {
  effect_strategy->computeEffects(this);
}

void Experiment::recalculateEverything() {
  
  this->calculateStatistics();
  
  this->calculateEffects();
  
  this->calculateTests();
}


// Initialization and Resetting
// ------------------------


///
/// It allocates the necessary memory for the dependent variables.
///
/// @todo This should be done nicer! It's only being done like this at the moment
/// because I need to update each dv's id.
void Experiment::initResources() {

  dvs_.resize(setup.ng());
  
  std::for_each(dvs_.begin(), dvs_.end(), [i = 0](auto &dv) mutable {
    dv.id_ = i++;
  });
}


///
/// This cleanup the Experiment, and reallocate its memory again. Technically, preparing
/// the experiment for a new run.
///
void Experiment::reset() {
  clear();
  initResources();
}

///
/// It clears every DVs individually, and also sort them back into the correct order.
///
/// @todo I think this is a bad implementation, and I should probably just discard the
/// list of DVs and recreate them for the new Experiment, which is probably safer!
/// Something like `dvs_.clear()`
///
void Experiment::clear() {

  for (auto &dv : dvs_) {
    dv.clear();
  }

  // This is strange, and I don't want it! It's here because Policy might mess up the
  // order of DVs when it runs some of the function on a group of them. I think if I use
  // std::reference_wrapper<T> I can avoid this but that needs some extra work.
  std::sort(dvs_.begin(), dvs_.end(),
            [](const auto &l, const auto &r) { return l.id_ < r.id_; });
  
  
  has_candidates = false;
  is_hacked = false;
  is_published = false;
  
}


// Operators
// ---------

///
/// These operators are returning the correct group, even if the group list is
/// not sorted.
///
/// @todo I'm not the fan of this `find_if` here, and I think I can do better.
/// @todo I think I can avoid this search if I use std::reference_wrapper
///
DependentVariable& Experiment::operator[](std::size_t idx) {
  if (idx > dvs_.size()) {
    throw std::invalid_argument("Index out of bound.");
  }
  
  auto g = std::find_if(dvs_.begin(), dvs_.end(), [&](auto &g) -> bool {return g.id_ == idx; });
  return *g;
}

///
const DependentVariable& Experiment::operator[](std::size_t idx) const {
  if (idx > dvs_.size()) {
    throw std::invalid_argument("Index out of bound.");
  }
  
  auto g = std::find_if(dvs_.cbegin(), dvs_.cend(), [&](auto &g) -> bool {return g.id_ == idx; });
  return *g;
}

// Getter / Setter / Status Query
// ----------------------------


/// It sets the overall hacked status of the experiment to status.
///
/// @attention Setting this to `true` will make the `isHacked()` to return `true`.
/// Basically, this overrules the status of the dependent variables, but it doesn't
/// overwrite them!
void Experiment::setHackedStatus(const bool status) {
  is_hacked = status;
}

///
void Experiment::setPublishedStatus(const bool status) {
  is_published = status;
}

///
void Experiment::setHackedStatusOf(const std::vector<size_t> &idxs, const bool status) {
  std::for_each(idxs.begin(), idxs.end(),
                [this, &status](auto &i){
                    this->dvs_[i].setHackedStatus(status);
                  });
}

///
void Experiment::setCandidateStatusOf(const std::vector<size_t> &idxs, const bool status) {
  has_candidates = true;
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
}

///
bool Experiment::isModified() const {
  return std::any_of(dvs_.begin(), dvs_.end(), [](auto &dv){return dv.isModified();});
}

///
bool Experiment::hasCandidates() const {
  return std::any_of(dvs_.begin(), dvs_.end(), [](auto &dv){return dv.isCandidate();});
}

///
size_t Experiment::nCandidates() const {
  return std::count_if(dvs_.begin(), dvs_.end(), [](auto &dv){return dv.isCandidate();});
}

int Experiment::nCovariants() const {
  return n_covariants;
}

bool Experiment::hasCovariants() const {
  return (n_covariants != 0);
}

///
bool Experiment::isPublished() const {
  return is_published;
}
