//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include "spdlog/spdlog.h"

#include "Researcher.h"

using namespace sam;

ResearcherBuilder Researcher::create(std::string name) {
  return ResearcherBuilder(name);
}

void Researcher::letTheHackBegin() {
  
  for (auto &set : workflow) {
    
    Experiment copiedExpr = *experiment;
    
    std::visit(overload {
        [&](HackingSet& hset) {
          for (auto &method : hset) {
            (*method)(&copiedExpr);
            copiedExpr.is_hacked = true;
          }
        },
        [&](PolicyChain& policy) {
          decision_strategy->operator()(&copiedExpr, policy);
          if (decision_strategy->hasSubmissionCandidate() && not decision_strategy->isStillHacking()){
            return;
          }
        }
    }, set);
    
  }
  
}

///
/// Iterating over the registrated methods and run them on the current
/// experiment.
///
/// \note       This has a very similar implemention to the `hack()` but it
/// doesn't
///             perform any of the secondary checks.
///
/// \bug        I think there is a possible bug here, since pre-processing
/// methods
///             can be much more aggresive, they can cause some issues when it
///             comes to calculating statistics.
///
void Researcher::preProcessData() {

//  static NoDecision no_decision = NoDecision();

  experiment->calculateStatistics();

  for (auto &method : pre_processing_methods) {

    (*method)(experiment);
  }
}

void Researcher::research() {

  spdlog::debug("Executing the Research Workflow!");
  
  experiment->generateData();
  
  if (is_pre_processing) {
    spdlog::debug("Pre-processing");

    preProcessData();
  }
  
  computeStuff();
  
  /// Checking the Initial Policies
  decision_strategy->operator()(experiment, decision_strategy->initial_decision_policies);
  
  /// Checking if hacknig is necessary
  if (isHacker() && not decision_strategy->hasSubmissionCandidate()) {
    letTheHackBegin();
  }
  
  /// Checking if we are Patient, if so, going to select among those
  if (not decision_strategy->hasSubmissionCandidate()){
    decision_strategy->operator()(decision_strategy->final_decision_policies);
  }
  
  /// Checking the Submission Policies
  if (decision_strategy->willBeSubmitting(decision_strategy->submission_policies)) {
    journal->review(decision_strategy->current_submission_candidate);
  }
  /// else: She didn't find anything, and nothing will be submitted to the Journal.
  ///       Current experiment will be discarded...
  
}
