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
//===----------------------------------------------------------------------===// 

#include "spdlog/spdlog.h"

#include "Researcher.h"

#include <optional>
#include <utility>

using namespace sam;

///
ResearcherBuilder Researcher::create(std::string name) {
  return ResearcherBuilder(std::move(name));
}

/// This uses HackingWorkflow to sequentially apply sets of
/// hacking → selection → decision on the available Experiment. Before applying
/// each hacking strategy, researcher asks isCommittingToTheHack() to decide on 
/// whether or not it is going to commit to a hack, if not, the rest of the
/// set will be ignored, and researcher continues with the next set, if
/// available.
///
/// @return     Returns `true` if any of the decision steps passes, it returns 
///             `false` indicating that none of the selection → decisions were 
///             successful.
std::optional<SubmissionPool>
Researcher::hackTheResearch() {

  spdlog::debug("Initiate the Hacking Procedure...");
  
  for (auto &hacking_group : hacking_workflow) {

    Experiment copy_of_experiment = *experiment;

    // It indicates whether or not the hacking is successful, if so, we skip the
    // remaining of the methods
    bool stopped_hacking{false};

    for (auto &h_set : hacking_group) {

      // This is a handy flag to propagate the information out of the std::visit
      // It indicates whether or not the Researcher has committed to the hack or
      // not.
      bool has_committed{true};

      std::optional<SubmissionPool> hacked_subs;

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
                  hacked_subs = research_strategy->selectOutcomeFromExperiment(
                      &copy_of_experiment, selection_policies);
                },
                [&](PolicyChain &decision_policy) {
                  // Performing a Decision

                  spdlog::trace(
                      "Checking whether we are going to continue hacking?");

                  if (hacked_subs and
                      !research_strategy->willContinueHacking(hacked_subs, decision_policy)) {
                    spdlog::trace("Done Hacking!");
                    stopped_hacking = true;
                  }

                  spdlog::trace("Continue Hacking...");
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
          return hacked_subs;
        }
      }
    }
  }

  // All hacking strategies are exhausted, and we didn't find anything, so, we
  // leave and notify the researcher by returning `false`.
  spdlog::trace("No more hacking strategies...");
  return std::nullopt;
}

///
/// Based on the provided settings, this re-selects, re-arranges, and shuffles 
/// the list of hacking strategies, and their corresponding parameters.
///
/// @note It worth mentioning that, this method doesn't randomize the internal
/// parameters of individual hacking strategies. Hacking strategies parameters
/// can be randomized only if their parameters are set to be a Parameter<T>.
///
void Researcher::randomizeHackingStrategies() {

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

  // For one last time, researcher checks to see if everything is ok before it 
  // continue with the submission process
  if (subs and research_strategy->willBeSubmitting(
                   subs, research_strategy->submission_decision_policies)) {
    spdlog::trace("To be submitted submission: {}",
                  candidate_submissions.value());

    // Decides whether the researcher follows through with the submission or
    // bails out and put her research into the drawer!
    if (Random::get<bool>(static_cast<float>(submission_probability()))) {
      journal->review(candidate_submissions.value());
    }
  }
}

///
/// This technically invokes the #probability_of_being_a_hacker, and
/// returns the outcome. The value then will be cast-ed to a boolean to 
/// determine whether the Researcher is going to start the hacking procedure or 
/// not, ie., calling or skipping the hackTheResearch().
///
/// @note Note that the #probability_of_being_a_hacker is called through it's 
/// call operator(). This guarantees that its value is being randomized 
/// _only if_ it contains a distribution.
///
bool Researcher::isHacker() {
  return Random::get<bool>(
      static_cast<float>(probability_of_being_a_hacker()));
}

///
/// Similar to the isHacker() method, this returns a boolean indicating whether 
/// or not the Researcher will commit to the given hacking method. The 
/// probability of commitment to a hacking strategy is being calculated from the
/// value of #probability_of_committing_a_hack.
/// 
bool Researcher::isCommittingToTheHack(HackingStrategy *hs) {
  return std::visit(
      overload{[&](float &p) { return Random::get<bool>(p); },
               [&](std::string &s) {
                 if (s == "prevalence") {
                   return Random::get<bool>(hs->prevalence());
                 }
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
/// The internal of the method is based on sequential application of 
/// selection → decision sequences on the experiment. Throughout the process, 
/// the researcher keeps the list of candidate 
/// submissions, #candidate_submissions, up-to-date. This starts by the initial
/// selection, and following though with the hacking, and stashing selection. At
/// the end of each replication, if any, researcher collects the last submission
/// candidate and head to perform a new experiment. After performing all the 
/// replications (or skipping some), researcher perform a final selection → 
/// decision on the list of #replicated_submissions and select her final 
/// submission to be submitted to the Journal.
///
void Researcher::research() {

  spdlog::debug("Executing the Research Workflow!");

  // Prepare the Research
  // ====================

  experiment->setup.randomize();
  experiment->reset();

  // Looping over the number of replications...
  for (int rep{0}; rep < experiment->setup.nreps(); ++rep) {

    experiment->repid = rep;

    spdlog::trace("–––––––––––––––––––");
    spdlog::trace("Replication #{} ↓", rep);

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


    candidate_submissions = 
      research_strategy->selectOutcomeFromExperiment(
        experiment.get(), 
        research_strategy->initial_selection_policies);

    stashed_submissions = research_strategy->stashedSubmissions();

    // _Will Start Hacking_ Selection → Decision Sequence
    // --------------------------------------------------

    // If Researcher is a hacker, and it decides to start hacking — based on
    // the current submission candidates list —, then, we are going to the
    // hacking procedure!
    if (isHacker() and research_strategy->willStartHacking(candidate_submissions)){

      candidate_submissions = hackTheResearch();
      stashed_submissions = research_strategy->stashedSubmissions();

    } 

    // If we don't have a candidate yet, then we look into the stashed pile
    if ((not candidate_submissions) and stashed_submissions) {
      spdlog::trace("Selecting between Stashed Submissions: {}",
                    stashed_submissions.value());

      candidate_submissions = 
        research_strategy->selectOutcomeFromPool(
          stashed_submissions.value(),
          research_strategy->between_stashed_selection_policies);
    }


    // _Will be Submitting_ Selection → Decision Sequence
    // --------------------------------------------------
    if (research_strategy->willBeSubmitting(
            candidate_submissions,
            research_strategy->submission_decision_policies)) {
      // If we have a submit-able candidate, then we collect it
      // Collecting in this case means that selected submissions will be added
      // to the current replication's outcome

      spdlog::trace("Final Submission Candidates: {}",
                    candidate_submissions.value());
      
      replication_submissions.insert(
               replication_submissions.end(),
               candidate_submissions.value().begin(),
               candidate_submissions.value().end());
    
      
      spdlog::trace("Collection of Submissions from Replications: {}",
                    replication_submissions);
    }

    // _Will Continue Replicating_ Decision
    // ------------------------------------
    if (not research_strategy->willContinueReplicating(replication_submissions)) {
      break;
    }

    // Reset the research_strategy before starting a new replication
    research_strategy->reset();
  }

  // BetweenReplications Selection → Decision Sequence
  // -------------------------------------------------
  spdlog::trace("__________");
  spdlog::trace("→ Choosing Between Replications");
  if (not replication_submissions.empty()) {
    candidate_submissions = research_strategy->selectOutcomeFromPool(
        replication_submissions, research_strategy->between_reps_policies);
  }

  // Will be Submitting Selection → Decision Sequence
  // ------------------------------------------------
  submitTheResearch(candidate_submissions);

  // Clean up everything, before starting a new research
  this->reset();
}

///
/// Based on the given priority randomizes the hacking workflow.
///
/// @param      hw        The hacking workflow
/// @param      priority  The sorting priority
///
/// @note This will return a critical error if either of the hacking strategies doesn't
/// have the appropriate parameters defined
///
void Researcher::reorderHackingStrategies(HackingWorkflow &hw,
                                          std::string &priority) {
  if (priority.empty()) {
    return;
  }

  for (auto &group : hw) {
    try {

      if (priority == "random") {
        Random::shuffle(hw);
      } else if (priority == "asc(prevalence)") {
        std::sort(group.begin(), group.end(), [&](auto &h1, auto &h2) {
          return std::get<0>(h1[0])->prevalence() <
                 std::get<0>(h2[0])->prevalence();
        });
      } else if (priority == "desc(prevalence)") {
        std::sort(group.begin(), group.end(), [&](auto &h1, auto &h2) {
          return std::get<0>(h1[0])->prevalence() >
                 std::get<0>(h2[0])->prevalence();
        });
      } else if (priority == "asc(defensibility)") {
        std::sort(group.begin(), group.end(), [&](auto &h1, auto &h2) {
          return std::get<0>(h1[0])->defensibility() <
                 std::get<0>(h2[0])->defensibility();
        });
      } else if (priority == "desc(defensibility)") {
        std::sort(group.begin(), group.end(), [&](auto &h1, auto &h2) {
          return std::get<0>(h1[0])->defensibility() >
                 std::get<0>(h2[0])->defensibility();
        });
      } else /* sequential */ {
        spdlog::critical("Invalid argument!");
        exit(1);
      }

    } catch (const std::bad_optional_access& e) {
      spdlog::critical("Cannot sort the hacking strategies based on the given priority.");
      spdlog::critical("Make sure that the defensibility and/or prevalence are defined for every hacking strategy.");
      exit(1);
    }
  }
}
