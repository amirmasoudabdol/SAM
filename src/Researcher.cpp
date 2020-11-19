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
  
  spdlog::debug("Initiate the Hacking Procedure...");
  
  /// @todo Does this copy even make any sense! I don't think so!
  Experiment copy_of_experiment = *experiment;
  
  bool stopped_hacking {false};

  /// This is a handy flag for propogating the information out of the std::visit.
  bool has_committed;
  
  for (auto &h_set : h_workflow) {
    /// \todo Now that I have a set, I don't really need the has_commited variable!
  
    has_committed = true;
    
    for (auto &step : h_set) {
            
      /// In each step, we either run a hack or a policy
      std::visit(overload {

          [&](std::shared_ptr<HackingStrategy>& hacking_strategy) {
            /// Performing a Hack
            
            /// If we are not committed to the method, then, we just leave the entire
            /// set behind
            has_committed = isCommittingToTheHack(hacking_strategy.get());
            
            if (has_committed) {
              spdlog::trace("++++++++++++++++");
              spdlog::trace("→ Starting a new HackingSet");
              
              (*hacking_strategy)(&copy_of_experiment);
              copy_of_experiment.is_hacked = true;
            }
          },
          [&](PolicyChainSet& selection_policies) {
            /// Performing a Selection
            /// With PolicyChainSet we can look for different results
              decision_strategy->selectOutcomeFromExperiment(&copy_of_experiment, selection_policies);
          },
          [&](PolicyChain &decision_policy) {
              /// Performing a Decision
              /// With PolicyChain, we can only validate if a submission passes all the criteria
              spdlog::trace("Checking whether we are going to continue hacking or not?");
              if (!decision_strategy->willContinueHacking(&copy_of_experiment, decision_policy)) {
                spdlog::trace("Done Hacking!");
                stopped_hacking = true;
              }else{
                /// Since I'm going to continue hacking, I'm going to reset the candidate because it should be
                /// evaluated again, and I've already performed a selection, I'm going to reset the selected
                /// submission.
                /// @bug, this could possibly cause some confusion! In cases where I only have one strategy and
                /// wants to be done after it, this role discard the last selection.
                /// @workaround, the current workaround is to select from the stash using between_hacking_selection
                decision_strategy->submission_candidate.reset();
                spdlog::trace("Continue Hacking...");
              }
          }
        
      }, step);
      
      if (not has_committed) {
        // If we haven't commited to the method, we leave the rest behind too and go for the
        // next method!
        break;
      }
      
      /// We leave the workflow when we have a submission, and it also passes the decision policy
      if (stopped_hacking) {
        return;
      }
      
    }
  }
  
}

void Researcher::randomizeParameters() {
  
  if (reselect_hacking_strategies_after_every_simulation) {
    
    /// Clearing the list
    h_workflow.clear();
    
    /// Shuffling the original list
    Random::shuffle(original_workflow.begin(), original_workflow.end());
    h_workflow = original_workflow;
    
    /// Sorting based on the given selection criteria
    reorderHackingStrategies(h_workflow, hacking_selection_priority);
    h_workflow.resize(n_hacks);

    /// Reordering based on the given execution order
    reorderHackingStrategies(h_workflow, hacking_execution_order);
    
    /// @note Then I need a for-loop to randomize each strategy
    ///      - I think this might not be necessary as Parameter handles the randomization
    ///         automatically
  }
  
}

///
/// Iterating over the registrated methods and run them on the current
/// experiment.
///
/// \note This has a very similar implemention to the `hack()` but it
/// doesn't perform any of the secondary checks.
void Researcher::preProcessData() {

  experiment->calculateStatistics();

  for (auto &method : pre_processing_methods) {
    method->operator()(experiment.get());
  }
}

/// \brief Checking the final submission and submitting it to
/// the Journal.
///
/// \todo Maybe I should pass the `final_submission` to this, and don't rely on it
/// reading it from the `decision_strategy`
void Researcher::checkAndsubmitTheResearch(const std::optional<Submission> &sub) {
  
  spdlog::debug("Checking whether the Final Submission is going to be submited to Journal...");
  
  if (sub and
      decision_strategy->willBeSubmitting(sub, decision_strategy->submission_decision_policies)) {
    spdlog::trace("To be submitted submission: ");
    spdlog::trace("\t{}", decision_strategy->submission_candidate.value());
    
    if (Random::get<bool>(submission_probability))
      journal->review(decision_strategy->submission_candidate.value());
  }
  
}

/// \todo this should not draw a new value every time, and should draw once
/// and keep reporting that one.
/// \updated Should it thought?
bool Researcher::isHacker() {
  
//  return std::visit(overload {
//    [&](double &p) {
//      return Random::get<bool>(p);
//    },
//    [&](Distribution &dist) {
//      return Random::get<bool>(Random::get(dist));
//    }
//  }, probability_of_being_a_hacker);
//
  return Random::get<bool>(static_cast<double>(probability_of_being_a_hacker));
  
}

bool Researcher::isCommittingToTheHack(HackingStrategy *hs) {
  return std::visit(overload {
    [&](double &p) {
      return Random::get<bool>(p);
    },
    [&](std::string &s) {
      if (s == "prevalence") {
        return Random::get<bool>(hs->prevalence());
      } else /* (s == "defensibility") */ {
        return Random::get<bool>(hs->defensibility());
      }
    },
    [&](Distribution &dist) {
      return Random::get<bool>(Random::get(dist));
    },
    [&](std::unique_ptr<HackingProbabilityStrategy> &hps) {
      return Random::get<bool>(hps->estimate(experiment.get()));
    }
  }, probability_of_committing_a_hack);
}

/// \brief  Executing the research workflow
///
/// This is the main routine that the Researcher execute.
void Researcher::research() {

  spdlog::debug("Executing the Research Workflow!");
  
  /// Preparing the experiment for a new Research
  
  // Incrementing the experiment Id
  experiment->exprid++;
  
  // Randomizing the experiment parameters if necessary
  experiment->setup.randomizeTheParameters();
  experiment->initExperiment();
  

  // Performing maximum `nreps` replications
  for (int rep{0}; rep < experiment->setup.nreps(); ++rep){
    
    experiment->repid = rep;
    
    {
      spdlog::trace("–––––––––––––––––––");
      spdlog::trace("Replication #{} ↓", rep);
    }
  
    experiment->generateData();
    
    if (is_pre_processing) {
      spdlog::debug("Initiating the Pre-processing Procedure...");

      preProcessData();
    }
    
    // Computing the statistics, effects, etc.
    computeStuff();
    
    /// -----------------
    /// Initial SELECTION
    spdlog::trace("→ Checking the INITIAL policies");
    decision_strategy->selectOutcomeFromExperiment(experiment.get(),
                                  decision_strategy->initial_selection_policies);
    
    /// ----------------------
    /// WillBeHacking DECISION
    if (isHacker()) {
      if (decision_strategy->willStartHacking()) {
        /// If we are a hacker, and we decide to start hacking — based on the current submission —,
        /// then, we are going to the hacking procedure!
        
          letTheHackBegin();

      }
    }
    
    /// ------------------------------
    /// BetweenHackedOutcome SELECTION
    if (not decision_strategy->submission_candidate and
        not decision_strategy->submissions_pool.empty()){
      /// If the pool of viable submissions is not empty, then we have to choose between them!
      /// Otherwise, we don't have to look into the pile!
      
      {
        spdlog::trace("→ Selecting between Hacked and Stashed Outcome!");
        for (auto &sub : decision_strategy->submissions_pool)
          spdlog::trace("\t{}", sub);
        spdlog::trace("-----^");
        // assert(!decision_strategy->between_hacks_selection_policies.empty() && "Research doesn't know how to select a submission from hacked submissions!");
      } // Logging
      
      decision_strategy->selectOutcomeFromPool(decision_strategy->submissions_pool,
                                               decision_strategy->between_hacks_selection_policies);
    }

    
    /// ---------------------
    /// Replication Stashings
    if (decision_strategy->willBeSubmitting(decision_strategy->submission_candidate, decision_strategy->submission_decision_policies)) {
      /// If we have a submittable candidate, then we collect it
      
      {
        spdlog::trace("Final Submission Candidate: ");
        spdlog::trace("\t{}", decision_strategy->submission_candidate.value());
      } // Logging
      
      submissions_from_reps.push_back(decision_strategy->submission_candidate.value());
      
    }
    
    /// ----------------------------------
    /// Will Continue Replicating DECISION
    if(!decision_strategy->willContinueReplicating(decision_strategy->will_continue_replicating_decision_policy)) {
      break;
    }
    
    // Reset also clear the decision strategy
    decision_strategy->reset();
    
  }
  
  /// BetweenReplications SELECTION
  if (submissions_from_reps.size() > 1) {
    /// If we have done more than one replication, then we have to select between them
    {
      spdlog::trace("__________");
      spdlog::trace("→ Choosing Between Replications");
      assert(!decision_strategy->between_reps_policies.empty() && "Research doesn't know how to select between submissions!");
    }
    decision_strategy->selectOutcomeFromPool(submissions_from_reps, decision_strategy->between_reps_policies);
  }else{
    /// If we did only one replication, then if there is anything, that's our final submission
    if (submissions_from_reps.size() == 1) {
      decision_strategy->submission_candidate = submissions_from_reps.front();
    }
  }
  
  checkAndsubmitTheResearch(decision_strategy->submission_candidate);
  
  decision_strategy->reset();
  experiment->clear();
  
  submissions_from_reps.clear();
  
}


/// Re-order the hacking strategies according the priority
void Researcher::reorderHackingStrategies(HackingWorkflow &hw, std::string priority) {
  if (priority == "random") {
    Random::shuffle(hw);
  } else if (priority == "asc(prevalence)") {
    std::sort(hw.begin(), hw.end(), [&](auto &h1, auto &h2){
      return std::get<0>(h1[0])->prevalence() < std::get<0>(h2[0])->prevalence();
    });
  } else if (priority == "desc(prevalence)") {
    std::sort(hw.begin(), hw.end(), [&](auto &h1, auto &h2){
      return std::get<0>(h1[0])->prevalence() > std::get<0>(h2[0])->prevalence();
    });
  } else if (priority == "asc(defensibility)") {
    std::sort(hw.begin(), hw.end(), [&](auto &h1, auto &h2){
      return std::get<0>(h1[0])->defensibility() < std::get<0>(h2[0])->defensibility();
    });
  } else if (priority == "desc(defensibility)") {
    std::sort(hw.begin(), hw.end(), [&](auto &h1, auto &h2){
      return std::get<0>(h1[0])->defensibility() > std::get<0>(h2[0])->defensibility();
    });
  } else /* sequential */ {
    throw std::invalid_argument("Invalid argument!");
  }
}
