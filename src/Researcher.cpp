//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include "spdlog/spdlog.h"

#include "Researcher.h"

using namespace sam;

ResearcherBuilder Researcher::create(std::string name) {
  return ResearcherBuilder(name);
}

void Researcher::hack() {

  for (auto &set : hacking_strategies) {

    // For each set, we make a copy of the experiment and apply the given
    // set of methods over each other.

    Experiment copiedExpr = *experiment;

    for (auto &method : set) {

      (*method)(&copiedExpr);
      copiedExpr.is_hacked = true;

      decision_strategy->verdict(copiedExpr, DecisionStage::DoneHacking);

      // If the researcher statisfied, hacking routine will be stopped
      if (!decision_strategy->isStillHacking()) {
        break;
      }
      
    }
  }
  
}


void Researcher::letTheHackBegin() {
  
  for (auto &set : workflow) {
    
    Experiment copiedExpr = *experiment;
    
    std::visit(overload{
        [&](HackingSet& hset) {
          for (auto &method : hset) {

            (*method)(&copiedExpr);
            copiedExpr.is_hacked = true;
            
          }
        },
        [&](PolicyChain& policy) {
          decision_strategy->operator()(&copiedExpr, policy);
          if (decision_strategy->already_found_something){
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

///
/// Prepares the research by cleaning up the memeory, randomizing the
/// ExperimentSetup parameters, allocating data and finally generating the data
/// using the DataGenStrategy
///
void Researcher::prepareResearch() {

  // TODO: Randomize if necessary

  // TODO: Maybe I need a `reset` or `clean` method for Experiment

  spdlog::debug("Generating Data");

  // Generating data using the dataStrategy
  experiment->generateData();

  // Performing the Pre-processing if any
  if (is_pre_processing) {
    spdlog::debug("Pre-processing");

    preProcessData();
  }
}

/// Performs the research by calculating the statistics, calculating the
/// effects, and running the test. In the case where the researcher is a hacker,
/// the researcher will apply the hacking methods on the `experiment`.
void Researcher::performResearch() {

  experiment->calculateStatistics();

  experiment->calculateEffects();

  spdlog::debug("Running Test");

  experiment->runTest();

  if (decision_strategy->verdict(*experiment, DecisionStage::Initial)
          .isStillHacking() &&
      isHacker()) {
    hack();
  }
}

///
/// Prepares the submission record by asking the `decisionStrategy` to pick his
/// prefered submission record from the list of available submission,
/// `submissionsList`. After than, it'll submit the submission record to the
/// selected `journal`.
///
void Researcher::publishResearch() {

  // Ask for the final decision
  decision_strategy->verdict(*experiment, DecisionStage::Final);

  // Setting researcher's submission record
  submission_record = decision_strategy->final_submission;

  // Submit the final submission to the Jouranl
  if (decision_strategy->willBeSubmitting())
    journal->review(decision_strategy->final_submission);
}

void Researcher::research() {

  // This needs to be reformulated to execute the _workflow_
  
  
  spdlog::debug("Executing the Research Workflow!");
  
  experiment->generateData();
  
  if (is_pre_processing) {
    spdlog::debug("Pre-processing");

    preProcessData();
  }
  
  computeStuff();

  if (decision_strategy->verdict(*experiment, DecisionStage::Initial)
          .isStillHacking() &&
      isHacker()) {
    hack();
  }
  
  decision_strategy->operator()(experiment, decision_strategy->initial_decision_policies);
  
  if (!decision_strategy->already_found_something) {
    letTheHackBegin();
  }
  
  // Ask for the final decision
//  decision_strategy->verdict(*experiment, DecisionStage::Final);

  // Setting researcher's submission record
//  submission_record = decision_strategy->final_submission;
  
  
  if (!decision_strategy->already_found_something){
    // Run Final Decision
    decision_strategy->operator()(decision_strategy->final_decision_policies);
  }
  
  if (!decision_strategy->already_found_something) {
    /// Run Submission Decision
    decision_strategy->operator()(decision_strategy->current_submission, decision_strategy->submission_policies);
  }
  
//
//  // Submit the final submission to the Jouranl
//  if (decision_strategy->willBeSubmitting())
//    journal->review(decision_strategy->final_submission);
  
  if (!decision_strategy->already_found_something){
      journal->review(decision_strategy->current_submission);
  }
  
  
}
