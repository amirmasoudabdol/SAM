//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include "spdlog/spdlog.h"

#include "Researcher.h"

using namespace sam;

ResearcherBuilder Researcher::create(std::string name) {
  return ResearcherBuilder(name);
}

bool Researcher::letTheHackBegin() {

  spdlog::debug("Initiate the Hacking Procedure...");

  /// @todo Does this copy even make any sense! I don't think so!
  Experiment copy_of_experiment = *experiment;

  bool stopped_hacking{false};

  /// This is a handy flag for propagating the information out of the
  /// std::visit.
  bool has_committed;

  for (auto &h_set : h_workflow) {

    has_committed = true;

    for (auto &step : h_set) {

      /// In each step, we either run a hack or a policy
      std::visit(
          overload{

              [&](std::shared_ptr<HackingStrategy> &hacking_strategy) {
                /// Performing a Hack

                /// If we are not committed to the method, then, we just leave
                /// the entire set behind
                has_committed = isCommittingToTheHack(hacking_strategy.get());

                if (has_committed) {
                  spdlog::trace("++++++++++++++++");
                  spdlog::trace("→ Starting a new HackingSet");

                  (*hacking_strategy)(&copy_of_experiment);
                  copy_of_experiment.is_hacked = true;
                }
              },
              [&](PolicyChainSet &selection_policies) {
                /// Performing a Selection
                /// With PolicyChainSet we can look for different results
                /// @note This will overwrite the submission_candidates, and if
                /// stashing, it'll stash everything into stashed_submissions
                decision_strategy->selectOutcomeFromExperiment(
                    &copy_of_experiment, selection_policies);
              },
              [&](PolicyChain &decision_policy) {
                /// Performing a Decision
                /// With PolicyChain, we can only validate if a submission
                /// passes all the criteria
                spdlog::trace(
                    "Checking whether we are going to continue hacking?");
                spdlog::trace("Looking for: {}", decision_policy);
                if (!decision_strategy->willContinueHacking(&copy_of_experiment,
                                                            decision_policy)) {
                  spdlog::trace("Done Hacking!");
                  stopped_hacking = true;
                } else {
                  /// Since I'm going to continue hacking, I'm going to reset
                  /// the candidate because it should be evaluated again, and
                  /// I've already performed a selection, I'm going to reset the
                  /// selected submission.
                  /// @bug, this could possibly cause some confusion! In cases
                  /// where I only have one strategy and wants to be done after
                  /// it, this role discard the last selection.
                  /// @workaround, the current workaround is to select from the
                  /// stash using between_hacking_selection
                  decision_strategy->submission_candidates.reset();
                  spdlog::trace("Continue Hacking...");
                }
              }

          },
          step);

      /// If we haven't committed to the current method, we skip the following
      /// selection → decision sequence as well
      if (not has_committed) {
        break;
      }

      /// We leave the workflow when we have a submission, and it also passes
      /// the decision policy
      if (stopped_hacking) {
        return true;
      }
    }
  }

  spdlog::trace("No more hacking strategies...");
  return false;
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
    ///      - I think this might not be necessary as Parameter handles the
    ///      randomization
    ///         automatically
  }
}

///
/// Iterating over the registered methods and run them on the current
/// experiment.
///
/// @note This has a very similar implementation to the `hack()` but it
/// doesn't perform any of the secondary checks.
void Researcher::preProcessData() {

  experiment->calculateStatistics();

  for (auto &method : pre_processing_methods) {
    method->operator()(experiment.get());
  }
}

/// @brief Checking the Submission candidates and submitting them to the Journal
///
/// This checks whether there is any submissions at all, then, it checks whether
/// `submission_decision_policies` have a hit, if so, it gives a green light to
/// Researcher to submit the list of submissions; otherwise, it discards the
/// list.
///
void Researcher::checkAndsubmitTheResearch(
    const std::optional<std::vector<Submission>> &subs) {

  spdlog::debug("Checking whether the Final Submission is going to be "
                "submitted to Journal...");

  if (subs and decision_strategy->willBeSubmitting(
                   subs, decision_strategy->submission_decision_policies)) {
    spdlog::trace("To be submitted submission: {}",
                  decision_strategy->submission_candidates.value());

    if (Random::get<bool>(submission_probability))
      journal->review(decision_strategy->submission_candidates.value());
  }
}

/// @brief  Determines whether or not the Researcher is a hacker
bool Researcher::isHacker() {

  return Random::get<bool>(static_cast<double>(probability_of_being_a_hacker));
}

bool Researcher::isCommittingToTheHack(HackingStrategy *hs) {
  return std::visit(
      overload{[&](double &p) { return Random::get<bool>(p); },
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
               }},
      probability_of_committing_a_hack);
}

/// @brief  Executing the research workflow
///
/// This is the main routine that the Researcher execute.
void Researcher::research() {

  spdlog::debug("Executing the Research Workflow!");

  /// Preparing the experiment for a new Research

  // Randomizing the experiment parameters if necessary
  experiment->setup.randomizeTheParameters();
  experiment->initExperiment();

  // Performing maximum `nreps` replications
  for (int rep{0}; rep < experiment->setup.nreps(); ++rep) {

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
    decision_strategy->selectOutcomeFromExperiment(
        experiment.get(), decision_strategy->initial_selection_policies);

    bool init_desc_succeed{false};
    std::optional<SubmissionPool> init_submissions;

    if (decision_strategy->submission_candidates) {
      init_desc_succeed = true;
      init_submissions = decision_strategy->submission_candidates;
    }

    /// ----------------------
    /// WillBeHacking DECISION

    /// This shenanigans tries to avoid a scenario that hacking is successful
    /// and SAM has to discard the stashed submissions!
    bool hacking_succeed;

    /// If Researcher is a hacker, and it decides to start hacking — based on
    /// the current submission —, then, we are going to the hacking procedure!
    if (isHacker() and decision_strategy->willStartHacking()) {

      hacking_succeed = letTheHackBegin();

      if (hacking_succeed) {
        spdlog::trace("Selecting between Hacked Submissions: ");
        spdlog::trace("{}", decision_strategy->submission_candidates.value());
        decision_strategy->selectOutcomeFromPool(
            decision_strategy->submission_candidates.value(),
            decision_strategy->between_stashed_selection_policies);
      } else {

        /// @todo Make `stashed_submissions` a optional like
        /// `submission_candidates`
        if (!decision_strategy->stashed_submissions.empty()) {
          spdlog::trace("Selecting between Stashed Submissions: ");
          spdlog::trace("{}", decision_strategy->stashed_submissions);
          decision_strategy->selectOutcomeFromPool(
              decision_strategy->stashed_submissions,
              decision_strategy->between_stashed_selection_policies);
        }
      }

    } else {
      if (init_desc_succeed)
        decision_strategy->submission_candidates = init_submissions;
      else if (!decision_strategy->stashed_submissions.empty()) {
        spdlog::trace("Selecting between Stashed Submissions: {}",
                      decision_strategy->stashed_submissions);
        decision_strategy->selectOutcomeFromPool(
            decision_strategy->stashed_submissions,
            decision_strategy->between_stashed_selection_policies);
      }
    }

    /// ---------------------
    /// Replication Stashing
    if (decision_strategy->willBeSubmitting(
            decision_strategy->submission_candidates,
            decision_strategy->submission_decision_policies)) {
      /// If we have a submittable candidate, then we collect it

      spdlog::trace("Final Submission Candidates: {}",
                    decision_strategy->submission_candidates.value());

      submissions_from_reps.insert(
          submissions_from_reps.end(),
          decision_strategy->submission_candidates.value().begin(),
          decision_strategy->submission_candidates.value().end());

      spdlog::trace("Collection of Submissions from Replications: {}",
                    submissions_from_reps);
    }

    /// ----------------------------------
    /// Will Continue Replicating DECISION
    if (!decision_strategy->willContinueReplicating(
            decision_strategy->will_continue_replicating_decision_policy)) {
      break;
    }

    // Reset the decision_strategy before starting a new replication
    decision_strategy->reset();
  }

  /// -----------------------------
  /// BetweenReplications SELECTION
  spdlog::trace("__________");
  spdlog::trace("→ Choosing Between Replications");
  decision_strategy->selectOutcomeFromPool(
      submissions_from_reps, decision_strategy->between_reps_policies);

  checkAndsubmitTheResearch(decision_strategy->submission_candidates);

  decision_strategy->reset();
  experiment->clear();

  submissions_from_reps.clear();
}

/// Re-order the hacking strategies according the priority
void Researcher::reorderHackingStrategies(HackingWorkflow &hw,
                                          std::string priority) {
  if (priority.empty())
    return;

  if (priority == "random") {
    Random::shuffle(hw);
  } else if (priority == "asc(prevalence)") {
    std::sort(hw.begin(), hw.end(), [&](auto &h1, auto &h2) {
      return std::get<0>(h1[0])->prevalence() <
             std::get<0>(h2[0])->prevalence();
    });
  } else if (priority == "desc(prevalence)") {
    std::sort(hw.begin(), hw.end(), [&](auto &h1, auto &h2) {
      return std::get<0>(h1[0])->prevalence() >
             std::get<0>(h2[0])->prevalence();
    });
  } else if (priority == "asc(defensibility)") {
    std::sort(hw.begin(), hw.end(), [&](auto &h1, auto &h2) {
      return std::get<0>(h1[0])->defensibility() <
             std::get<0>(h2[0])->defensibility();
    });
  } else if (priority == "desc(defensibility)") {
    std::sort(hw.begin(), hw.end(), [&](auto &h1, auto &h2) {
      return std::get<0>(h1[0])->defensibility() >
             std::get<0>(h2[0])->defensibility();
    });
  } else /* sequential */ {
    throw std::invalid_argument("Invalid argument!");
  }
}
