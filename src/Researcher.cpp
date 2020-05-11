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
  
  Experiment copy_of_experiment;
  for (auto &set : workflow) {
    
    if (std::holds_alternative<HackingSet>(set))
      copy_of_experiment = *experiment;
        
    std::visit(overload {

        [&](HackingSet& hset) {
          spdlog::debug("++++++++++++++++");
          spdlog::debug("→ Starting a new HackingSet");
          for (auto &method : hset) {
            (*method)(&copy_of_experiment);
            copy_of_experiment.is_hacked = true;
          }
        },
        [&](PolicyChainSet& policy) {
          /// FIXME: There is a problem here, where this operation overwrite the `final_submissino_candidate` even if
          /// I'm not going to hack anymore. I should be able to check the willBeHacking() before this.
          /// This is not a good solution, but let's do it for now
          
          /// NOW, the problem is, if I'm not going to continue hacking, I'm not also
          /// going to save stuff,
          if (not decision_strategy->willBeHacking(copy_of_experiment))
              decision_strategy->operator()(&copy_of_experiment, policy);
        }
    }, set);
    
    if (decision_strategy->hasSubmissionCandidate() and (not decision_strategy->willBeHacking(copy_of_experiment))){
      return;
    }
    
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

void Researcher::checkAndsubmitTheResearch() {
  if (decision_strategy->has_a_final_candidate
      and decision_strategy->willBeSubmitting(decision_strategy->submission_policies)) {
    spdlog::debug("To be submitted submission: ");
    spdlog::debug("\t{}", decision_strategy->final_submission_candidate);
    journal->review(decision_strategy->final_submission_candidate);
  }
}

void Researcher::research() {

  spdlog::debug("Executing the Research Workflow!");

  
  for (int rep{0}; rep < experiment->setup.nreps(); ++rep){
    
    spdlog::debug("–––––––––––––––––––");
    spdlog::debug("Replication #{} ↓", rep);
  
    experiment->generateData();
    
    if (is_pre_processing) {
      spdlog::debug("Pre-processing");

      preProcessData();
    }
    
    computeStuff();
    
    /// Checking the Initial Policies
    spdlog::debug("→ Checking the INITIAL policies");
    decision_strategy->operator()(experiment,
                                  decision_strategy->initial_decision_policies);
    
    /// Checking if hacknig is necessary
    if (isHacker() and
        (not decision_strategy->has_a_final_candidate) and
        decision_strategy->willBeHacking(*experiment)) {
      letTheHackBegin();
    }else if (experiment->setup.nreps() <= 1) {
      checkAndsubmitTheResearch();
      
      decision_strategy->reset();
      experiment->clear();
      decision_strategy->clearHistory();
      
      this->submissions_from_reps.clear();
      
      return;
    }
    
    /// Checking if we are Patient, if so, going to select among those
    if (not decision_strategy->submissions_pool.empty()
        and not decision_strategy->has_a_final_candidate){
      
      spdlog::debug("→ Checking the FINAL policies");
      for (auto &sub : decision_strategy->submissions_pool)
        spdlog::debug("\t{}", sub);
      spdlog::debug("-----^");
      
      assert(!decision_strategy->final_decision_policies.empty() && "Research doesn't know how to select a submission from hacked submissions!");
      
      decision_strategy->operator()(decision_strategy->submissions_pool,
                                    decision_strategy->final_decision_policies);
    }
     /// else
     ///  then we don't need to look into the pile
    
    /// Checking the Submission Policies
    /// If we have a final candidate, and it's submitable, we are saving it to the submissions_from_reps
    if (decision_strategy->has_a_final_candidate
        and decision_strategy->willBeSubmitting(decision_strategy->submission_policies)) {
      
      spdlog::debug("Final Submission Candidate: ");
      spdlog::debug("\t{}", decision_strategy->final_submission_candidate);
      
      last_submission_candidate = decision_strategy->final_submission_candidate;
      
      submissions_from_reps.push_back(last_submission_candidate);
      
    }
    /// else: She didn't find anything, and nothing will be submitted to the Journal.
    ///       Current experiment will be discarded...
    
    decision_strategy->clearHistory();
    decision_strategy->reset();
    experiment->clear();

    // TODO: Here I can check if I want to stop the replication process or not, based on some set of policies
    // .....
    // .......... break;
    
  }
  
  if (submissions_from_reps.size() > 1) {
    spdlog::debug("__________");
    spdlog::debug("→ Choosing Between Replications");
    assert(!decision_strategy->between_reps_policies.empty() && "Research doesn't know how to select between submissions!");
    decision_strategy->operator()(submissions_from_reps, decision_strategy->between_reps_policies);
  }else{
    if (submissions_from_reps.size() == 1) {
      decision_strategy->final_submission_candidate = submissions_from_reps.front();
      decision_strategy->has_a_final_candidate = true;
    }
  }
  
  checkAndsubmitTheResearch();
  
  decision_strategy->reset();
  experiment->clear();
  
  this->submissions_from_reps.clear();
  
}
