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
  
  Experiment copy_of_experiment = *experiment;
  
  bool found_something_and_done_hacking {false};
  
  for (auto &step : h_workflow) {
    /// In each step, we either run a hack or a policy
    std::visit(overload {

        [&](std::shared_ptr<HackingStrategy>& hacking_strategy) {
          /// Performing a Hack
          
          spdlog::debug("++++++++++++++++");
          spdlog::debug("→ Starting a new HackingSet");
          (*hacking_strategy)(&copy_of_experiment);
          copy_of_experiment.is_hacked = true;
        },
        [&](PolicyChainSet& selection_policies) {
          /// Performing a Selection
          /// With PolicyChainSet we can look for different results
          decision_strategy->selectOutcomeFromExperiment(&copy_of_experiment, selection_policies);
        },
        [&](PolicyChain &decision_policy) {
          /// Performing a Decision
          /// With PolicyChain, we can only validate if a submission passes all the criteria
          
          if (decision_strategy->willContinueHacking(decision_policy)) {
            found_something_and_done_hacking = true;
          }
        }
      
    }, step);
    
    /// We leave the workflow when we have a submission, and it also passes the decision policy
    if (found_something_and_done_hacking) {
      return;
    }
    
  }
  
}

///
/// Iterating over the registrated methods and run them on the current
/// experiment.
///
/// \note This has a very similar implemention to the `hack()` but it
/// doesn't perform any of the secondary checks.
///
/// \bug  I think there is a possible bug here, since pre-processing
/// methods can be much more aggresive, they can cause some issues when it
/// comes to calculating statistics.
///
void Researcher::preProcessData() {

  experiment->calculateStatistics();

  for (auto &method : pre_processing_methods) {

    (*method)(experiment);
  }
}

/// \brief Checking the final submission and submitting it to
/// the Journal.
///
/// \todo Maybe I should pass the `final_submission` to this, and don't rely on it
/// reading it from the `decision_strategy`
void Researcher::checkAndsubmitTheResearch(const std::optional<Submission> &sub) {
  
  if (sub and
      decision_strategy->willBeSubmitting(sub, decision_strategy->submission_decision_policies)) {
    spdlog::debug("To be submitted submission: ");
    spdlog::debug("\t{}", decision_strategy->submission_candidate.value());
    journal->review(decision_strategy->submission_candidate.value());
  }
  
}

/// \brief  Executing the research workflow
///
/// This is the main routine that the Researcher execute.
void Researcher::research() {

  spdlog::debug("Executing the Research Workflow!");

  // Performing maximum `nreps` replications
  for (int rep{0}; rep < experiment->setup.nreps(); ++rep){
    
    {
      spdlog::debug("–––––––––––––––––––");
      spdlog::debug("Replication #{} ↓", rep);
    }
  
    experiment->generateData();
    
    if (is_pre_processing) {
      spdlog::debug("Pre-processing");

      preProcessData();
    }
    
    // Computing the statistics, effects, etc.
    computeStuff();
    
    /// Checking the Initial Policies
    spdlog::debug("→ Checking the INITIAL policies");
    decision_strategy->selectOutcomeFromExperiment(experiment,
                                  decision_strategy->initial_selection_policies);
    
    
    if (isHacker() and
        decision_strategy->willStartHacking()) {
      /// If we are a hacker, and we decide to start hacking — based on the current submission —,
      /// then, we are going to the hacking procedure!
      
      letTheHackBegin();
      
    } else if (experiment->setup.nreps() <= 1) {
      /// If we only doing one replication, and we already have something,
      /// then we are done. We'll check the submission, and decide whether
      /// we want to submit it or not
      
      checkAndsubmitTheResearch(decision_strategy->submission_candidate);
      
      decision_strategy->reset();
      experiment->clear();
      
      this->submissions_from_reps.clear();
      
      return;
    }
    
    /// Checking if we are Patient, if so, going to select among those
    if (not decision_strategy->submissions_pool.empty()
        and not decision_strategy->submission_candidate){
      
      {
        spdlog::debug("→ Checking the FINAL policies");
        for (auto &sub : decision_strategy->submissions_pool)
          spdlog::debug("\t{}", sub);
        spdlog::debug("-----^");
      } // Logging
      assert(!decision_strategy->between_hacks_selection_policies.empty() && "Research doesn't know how to select a submission from hacked submissions!");
      
      decision_strategy->selectOutcomeFromPool(decision_strategy->submissions_pool,
                                    decision_strategy->between_hacks_selection_policies);
    }
     /// else
     ///  then we don't need to look into the pile
    
    /// Checking the Submission Policies
    /// If we have a final candidate, and it's submitable, we are saving it to the submissions_from_reps
    if (decision_strategy->submission_candidate
        and decision_strategy->willBeSubmitting(decision_strategy->submission_candidate, decision_strategy->submission_decision_policies)) {
      
      {
        spdlog::debug("Final Submission Candidate: ");
        spdlog::debug("\t{}", decision_strategy->submission_candidate.value());
      }
      
      last_submission_candidate = decision_strategy->submission_candidate.value();
      
      submissions_from_reps.push_back(last_submission_candidate);
      
    }
    /// else: She didn't find anything, and nothing will be submitted to the Journal.
    ///       Current experiment will be discarded...
    
    /// \todo: This is an ugly `if`, but it works, the problem is that WillBeSubmitting is not designed robust enough
    /// to handle this situation.
    /// \todo: this can move to the for-loop check
    if (decision_strategy->submission_candidate
        and (!decision_strategy->will_continue_replicating_decision_policy.empty()
        and decision_strategy->willBeSubmitting(decision_strategy->submission_candidate, decision_strategy->will_continue_replicating_decision_policy))) {
      break;
    }
    
    // Reset also clear the decision strategy
    decision_strategy->reset();
    
  }
  
  if (submissions_from_reps.size() > 1) {
    /// If we have done more than one replication, then we have to select between them
    {
      spdlog::debug("__________");
      spdlog::debug("→ Choosing Between Replications");
      assert(!decision_strategy->between_reps_policies.empty() && "Research doesn't know how to select between submissions!");
    }
    decision_strategy->selectOutcomeFromPool(submissions_from_reps, decision_strategy->between_reps_policies);
  }else{
    /// If we did only one replication, then if there is anything, that's our final submission
    if (submissions_from_reps.size() == 1) {
      /// \todo: The reason I have to do this is that all functions depends on internal states of
      /// DecisionStrategy, if everything is more functional, then I don't have to do this...
      /// \todo Specially this one is unnecessary, I've already cleaned up the decision_strategy, and has to
      /// do this only for the checkAndSubmit function
      decision_strategy->submission_candidate = submissions_from_reps.front();
    }
  }
  
  checkAndsubmitTheResearch(decision_strategy->submission_candidate);
  
  decision_strategy->reset();
  experiment->clear();
  
  submissions_from_reps.clear();
  
}
