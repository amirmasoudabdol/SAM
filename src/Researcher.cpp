//===-- Researcher.cpp - Researcher Implementation ------------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-01-25.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the implementation of the Researcher class. The majority
/// of the research is being done though the hackTheResearch() and research()
/// methods.
/// 
///
//===----------------------------------------------------------------------===// 

#include "spdlog/spdlog.h"

#include "Researcher.h"

using namespace sam;

ResearcherBuilder Researcher::create(std::string name) {
  return ResearcherBuilder(name);
}

/// It uses HackingWorkflow to sequentially apply sets of
/// hacking → selection → decision on the available Experiment. Before applying
/// each hacking strategy, researcher asks isCommittingToTheHack() to decide
/// whether or not it is going to commit to a hack, if not, the rest of the
/// set will be ignored, and researcher continues with the next set, if
/// available.
///
/// @return     Returns `true` if any of the decision steps passes, otherwise,
/// it returns `false` indicating that none of the selection → decisions were
/// successful
bool Researcher::hackTheResearch() {

  spdlog::debug("Initiate the Hacking Procedure...");

  Experiment copy_of_experiment = *experiment;

  // It indicates whether or not the hacking is successful, if so, we skip the
  // remaining of the methods
  bool stopped_hacking{false};

  // This is a handy flag to propagate the information out of the std::visit
  // It indicates whether or not the Researcher has committed to the hack or
  // not.
  bool has_committed;

  for (auto &h_set : hacking_workflow) {

    has_committed = true;

    for (auto &step : h_set) {

      // In each step, we either run a hack or a policy, HackingWorkflow is
      // ordered such that every hacking strategy is followed by a
      // selection → decision sequence
      std::visit(
          overload{

              [&](std::shared_ptr<HackingStrategy> &hacking_strategy) {
                // Performing a Hack

                // If we are not committed to the method, we leave the entire
                // set behind
                has_committed = isCommittingToTheHack(hacking_strategy.get());

                if (has_committed) {
                  spdlog::trace("++++++++++++++++");
                  spdlog::trace("→ Starting a new HackingSet");

                  // Applying the hack
                  (*hacking_strategy)(&copy_of_experiment);

                  copy_of_experiment.setHackedStatus(true);
                }
              },
              [&](PolicyChainSet &selection_policies) {
                // Performing a Selection

                // This will overwrite the submission_candidates, and if
                // stashing, it'll select and stash some of the outcomes to
                // into stashed_submissions
                research_strategy->selectOutcomeFromExperiment(
                    &copy_of_experiment, selection_policies);
              },
              [&](PolicyChain &decision_policy) {
                // Performing a Decision

                spdlog::trace(
                    "Checking whether we are going to continue hacking?");
                spdlog::trace("Looking for: {}", decision_policy);

                if (!research_strategy->willContinueHacking(&copy_of_experiment,
                                                            decision_policy)) {
                  spdlog::trace("Done Hacking!");
                  stopped_hacking = true;
                } else {
                  // Since researcher uses only one research_strategy and uses
                  // it throughout the research, I need to reset the
                  // submission_candidates and let it to start fresh with a new
                  // hacking strategy
                  //
                  // @todo this needs to be a more functional implementation.

                  research_strategy->submission_candidates.reset();
                  spdlog::trace("Continue Hacking...");
                }
              }

          },
          step);

      // If we haven't committed to the current method, we skip its
      // corresponding selection → decision sequence as well
      if (not has_committed) {
        break;
      }

      // We leave the workflow when we have a submission, ie., after successful
      // decision policy
      if (stopped_hacking) {
        return true;
      }
    }
  }

  // All hacking strategies are exhausted, and we didn't find anything, so, we
  // leave and notify the researcher by returning `false`.
  spdlog::trace("No more hacking strategies...");
  return false;
}

///
/// Based on the provided settings, this re-selects, re-arranges, and shuffles 
/// the list of hacking strategies, and their corresponding parameters.
///
/// @note It worth mentioning that, this method doesn't randomize the internal
/// parameters of individual hacking strategies. Hacking strategies parameters
/// can be randomized only if their parameters are set to be a Parameter<T>.
///
/// @todo I think the name can be more specific
void Researcher::randomizeParameters() {

  if (reselect_hacking_strategies_after_every_simulation) {

    // Clearing the list
    hacking_workflow.clear();

    // Shuffling the original list
    Random::shuffle(original_workflow.begin(), original_workflow.end());
    hacking_workflow = original_workflow;

    // Sorting based on the given selection criteria
    reorderHackingStrategies(hacking_workflow, hacking_selection_priority);
    hacking_workflow.resize(n_hacks);

    // Reordering based on the given execution order
    reorderHackingStrategies(hacking_workflow, hacking_execution_order);
  }
}

///
/// Iterating over the registered #pre_processing_methods, this applies all of
/// the them to the current experiment. Keep in mind that the pre-processing is 
/// done before any of the decision/hacking stages, and right after data
/// generation.
///
/// @note This has a very similar implementation to the `hackTheResearch()` but
/// it doesn't perform any of the secondary checks, and it does not incorporates
/// any of the selection → decision sequences.
///
/// @todo I think this can/need to be replaced with the notion of HackingStage
/// 
void Researcher::preProcessData() {

  experiment->calculateStatistics();

  for (auto &method : pre_processing_methods) {
    method->operator()(experiment.get());
  }
}

///
/// This checks whether there is any submissions at all, if so, it checks
/// whether `submission_decision_policies` have any hits, if so, it gives a
/// green light to Researcher to submit the list of submissions; otherwise, it
/// discards the list.
/// 
void Researcher::submitTheResearch(
    const std::optional<std::vector<Submission>> &subs) {

  spdlog::debug("Checking whether the Final Submission is going to be "
                "submitted to Journal...");

  if (subs and research_strategy->willBeSubmitting(
                   subs, research_strategy->submission_decision_policies)) {
    spdlog::trace("To be submitted submission: {}",
                  research_strategy->submission_candidates.value());

    if (Random::get<bool>(submission_probability)) {
      journal->review(research_strategy->submission_candidates.value());
    }
  }
}

///
/// This technically invokes the #probability_of_being_a_hacker, and
/// returns the outcome. The value then will be casted to a boolean to determine
/// whether the Researcher is going to start the hacking procedure or not, ie.,
/// calling or skipping the hackTheResearch().
///
/// @note Note that the #probability_of_being_a_hacker is called through it's 
/// call operator. This guarantees that its value is being randomized _only if_ 
/// it contains a distribution
/// 
bool Researcher::isHacker() {
  return Random::get<bool>(
      static_cast<double>(probability_of_being_a_hacker()));
}

///
/// Similar to the isHacker() method, this returns a boolean indicating whether 
/// or not the Researcher will commit to the given hacking method. The 
/// probability of commitment to a hacking strategy is being calculated from the
/// value of #probability_of_committing_a_hack.
/// 
bool Researcher::isCommittingToTheHack(HackingStrategy *hs) {
  return std::visit(
      overload{[&](double &p) { return Random::get<bool>(p); },
               [&](std::string &s) {
                 if (s == "prevalence") {
                   return Random::get<bool>(hs->prevalence());
                 }
                 /* else  (s == "defensibility") */
                 return Random::get<bool>(hs->defensibility());
               },
               [&](UnivariateDistribution &dist) {
                 return Random::get<bool>(Random::get(dist));
               },
               [&](std::unique_ptr<HackingProbabilityStrategy> &hps) {
                 return Random::get<bool>(hps->estimate(experiment.get()));
               }},
      probability_of_committing_a_hack);
}

///
/// This is the main routine of the Researcher. It is responsible for a few
/// things:
///
/// - Randomizing the Experiment, if necessary
/// - Initializing the Experiment
/// - Performing the research by:
///   - Generating the data
///   - Pre-processing the Data, if necessary
///   - Calculating the statistics
///   - Deciding whether to hack or not
///     - Perform the hack, if necessary
///   - Deciding whether to replicate the research
///     - Perform the replication, if necessary
///   - Evaluate the list of final submissions
///   - Submit the final submissions to the Journal, or discard the Experiment
///   - Clean up everything, and start a get ready for a new run
///
///
/// @todo This needs more doc!
///
void Researcher::research() {

  spdlog::debug("Executing the Research Workflow!");

  // Prepare the Research
  // ====================

  experiment->setup.randomize();
  experiment->reset();

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

    // _Initial_ Selection → Decision Sequence
    // -------------------------------------
    spdlog::trace("→ Checking the INITIAL policies");
    research_strategy->selectOutcomeFromExperiment(
        experiment.get(), research_strategy->initial_selection_policies);

    bool init_desc_succeed{false};
    std::optional<SubmissionPool> init_submissions;

    // Checking whether the Initial Selection was successful or not
    if (research_strategy->submission_candidates) {
      init_desc_succeed = true;
      init_submissions = research_strategy->submission_candidates;
    }

    // _Will Start Hacking_ Selection → Decision Sequence
    // --------------------------------------------------

    // This shenanigans tries to avoid a scenario that hacking is successful
    // and SAM has to discard the stashed submissions!
    bool hacking_succeed;

    // If Researcher is a hacker, and it decides to start hacking — based on
    // the current submission candidates list —, then, we are going to the
    // hacking procedure!
    if (isHacker() and research_strategy->willStartHacking()) {

      hacking_succeed = hackTheResearch();

      if (hacking_succeed) {

        spdlog::trace("Selecting between Hacked Submissions: ");
        spdlog::trace("{}", research_strategy->submission_candidates.value());
        research_strategy->selectOutcomeFromPool(
            research_strategy->submission_candidates.value(),
            research_strategy->between_stashed_selection_policies);
      } else {

        // If stashed is not empty AND hacking was not successful, then we are
        // going to select our candidates from the list of stashes submissions
        if (!research_strategy->stashed_submissions.empty()) {
          spdlog::trace("Selecting between Stashed Submissions: ");
          spdlog::trace("{}", research_strategy->stashed_submissions);
          research_strategy->selectOutcomeFromPool(
              research_strategy->stashed_submissions,
              research_strategy->between_stashed_selection_policies);
        }
      }

    } else {
      // If hacking has failed, or we didn't even go through hacking, we select
      // our submissions from the `init_submissions`, if not, we select from the
      // stashed submissions
      if (init_desc_succeed) {
        research_strategy->submission_candidates = init_submissions;
      } else if (!research_strategy->stashed_submissions.empty()) {
        spdlog::trace("Selecting between Stashed Submissions: {}",
                      research_strategy->stashed_submissions);
        research_strategy->selectOutcomeFromPool(
            research_strategy->stashed_submissions,
            research_strategy->between_stashed_selection_policies);
      }
    }

    // _Will be Submitting_ Selection → Decision Sequence
    // --------------------------------------------------
    if (research_strategy->willBeSubmitting(
            research_strategy->submission_candidates,
            research_strategy->submission_decision_policies)) {
      // If we have a submittable candidate, then we collect it
      // Collecting in this case means that selected submissions will be added
      // to the current replication's outcome

      if (research_strategy->submission_candidates) {
        spdlog::trace("Final Submission Candidates: {}",
                      research_strategy->submission_candidates.value());

        submissions_from_reps.insert(
            submissions_from_reps.end(),
            research_strategy->submission_candidates.value().begin(),
            research_strategy->submission_candidates.value().end());
      }

      spdlog::trace("Collection of Submissions from Replications: {}",
                    submissions_from_reps);
    }

    // _Will Continue Replicating_ Decision
    // ------------------------------------
    if (!research_strategy->willContinueReplicating(
            research_strategy->will_continue_replicating_decision_policy)) {
      break;
    }

    // Reset the research_strategy before starting a new replication
    research_strategy->reset();
  }

  // BetweenReplications Selection → Decision Sequence
  // -------------------------------------------------
  spdlog::trace("__________");
  spdlog::trace("→ Choosing Between Replications");
  research_strategy->selectOutcomeFromPool(
      submissions_from_reps, research_strategy->between_reps_policies);

  // Will be Submitting Selection → Decision Sequence
  // ------------------------------------------------
  submitTheResearch(research_strategy->submission_candidates);

  // Clean up everything, before starting a new research
  this->reset();
}

void Researcher::reorderHackingStrategies(HackingWorkflow &hw,
                                          std::string &priority) {
  if (priority.empty()) {
    return;
  }

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
