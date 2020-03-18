//
// Created by Amir Masoud Abdol on 2019-02-01.
//

#include <fmt/core.h>
#include <fmt/format.h>
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>

#include "DecisionStrategy.h"

using namespace sam;

/**
 * Pure deconstructor of DecisionStrategy.
 * This is necessary for proper deconstruction of derived
 * classes.
 */
DecisionStrategy::~DecisionStrategy(){
    // pure deconstructor
};

std::unique_ptr<DecisionStrategy>
DecisionStrategy::build(json &decision_strategy_config) {

  if (decision_strategy_config["_name"] == "ImpatientDecisionMaker") {

    auto params =
        decision_strategy_config.get<ImpatientDecisionMaker::Parameters>();
    return std::make_unique<ImpatientDecisionMaker>(params);

  } else if (decision_strategy_config["_name"] == "PatientDecisionMaker") {

    auto params =
        decision_strategy_config.get<PatientDecisionMaker::Parameters>();
    return std::make_unique<PatientDecisionMaker>(params);

  } else if (decision_strategy_config["_name"] == "HonestDecisionMaker") {

    return std::make_unique<HonestDecisionMaker>();

  } else {
    throw std::invalid_argument("Unknown DecisionStrategy");
  }
}

Submission DecisionStrategy::selectOutcome(Experiment &experiment) {

  /// We always start with the pre_registered_group, and if we find others
  /// results based on researchers' preference, then we replace it, and report
  /// that one.
  /// CHECK ME: I'm not sure if this is a good way of doing this...
  int selectedOutcome{0};

  pre_registered_group = experiment.setup.nd();

  this->complying_with_preference = true;

  int pset_inx{0};
  for (auto &policy_set : decision_policies) {

    /// These needs to be reset since I'm starting a new set of policies
    /// New policies will scan the set again!
    auto found_something{false};
    auto begin = experiment.groups_.begin() + experiment.setup.nd();
    auto end = experiment.groups_.end();

    for (auto &p : policy_set) {

      std::tie(found_something, begin, end) = checkThePolicy(begin, end, p);

      if (found_something) {
        /// If we have a hit, mainly after going to Min, Max, First, Random;
        /// then, we are done!
        spdlog::debug("Found something!");
        selectedOutcome = begin->id_;
        spdlog::debug("Policy: {}", pset_inx);
        return {experiment, selectedOutcome};
      } else {
        /// The range is empty! This only happens when Comp case cannot find
        /// anything with the given comparison. Then, we break out the loop, and
        /// move into the new set of policies
        if (begin == end) {
          spdlog::debug("Going to the next set of policies.");
          break; // Out of the for-loop, going to the next chain
        }
        /// else:
        ///     We are still looking!
      }
    }

    pset_inx++;
  }

  spdlog::debug("Found nothing!");
  selectedOutcome = 0;

  return {experiment, selectedOutcome};
}

/// This is often is being used by PatientDecisionMaker
Submission DecisionStrategy::selectBetweenSubmissions() {

  spdlog::debug("Selecting between collected submissions.");

  auto begin = submissions_pool.begin();
  auto end = submissions_pool.end();
  auto found_something{false};

  for (auto &policy : final_decision_policies) {

    std::tie(found_something, begin, end) = checkThePolicy(begin, end, policy);

    if (found_something) {
      spdlog::debug("Found something in the pile!");
      return *begin;
    } else {
      if (begin == end) {
        break;
      }
      /// else:
      ///     We are still looking. This happens when I'm testing a comparison
    }
  }

  spdlog::debug("Not happy!");

  /// It returns the last hacked results in the case not finding anything!
  return submissions_pool.back();
}

bool DecisionStrategy::willBeSubmitting() {

  /// This is signal from selectSubmission and it means that we couldn't find
  /// anything! So, we are not going to submit anything either
  if (current_submission.group_.id_ == 0) {
    return false;
  }

  // Checking whether all policies are returning `true`
  auto is_it_submittable = std::all_of(
      submission_policies.begin(), submission_policies.end(),
      [this](auto &policy) { return policy.second(this->current_submission); });

  return is_it_submittable;
}

/// Impatient decision maker keeps the initial study and stop the hacking
/// process if the results is already satistifactory.
///
/// @param experiment A reference to the experiment.
void ImpatientDecisionMaker::initDecision(Experiment &experiment) {

  // Preparing pools anyway
  experiments_pool.push_back(experiment);
  submissions_pool.push_back(current_submission);

  is_still_hacking = !willBeSubmitting();
}

/// Impatient decision maker check if the check current submission is
/// publishable or not, if not, it'll continue hacking... and will stop as soon
/// as it finds a publishable solution. This is different in the case of patient
/// decision maker for instance.
void ImpatientDecisionMaker::intermediateDecision(Experiment &experiment) {

  is_still_hacking = !willBeSubmitting();
}

void ImpatientDecisionMaker::afterhackDecision(Experiment &experiment) {

  /// TODO: I think this can be optimized, there are a few unnecessary calls
  /// here
  if (willBeSubmitting()) {
    experiments_pool.push_back(experiment);
    submissions_pool.push_back(current_submission);
  }

  is_still_hacking = !willBeSubmitting();
}

void ImpatientDecisionMaker::finalDecision(Experiment &experiment) {

  // I'm not sure if this is valid anymore considering the fact that there
  // is a willBeSubmitting() and policies in place now
  final_submission = submissions_pool.back();

  clearHistory();

  // Done Hacking...
  is_still_hacking = false;
}

ImpatientDecisionMaker &ImpatientDecisionMaker::verdict(Experiment &experiment,
                                                        DecisionStage stage) {

  current_submission = selectOutcome(experiment);

  switch (stage) {
  case DecisionStage::Initial:
    initDecision(experiment);
    break;
  case DecisionStage::WhileHacking:
    intermediateDecision(experiment);
    break;
  case DecisionStage::DoneHacking:
    afterhackDecision(experiment);
    break;
  case DecisionStage::Final:
    finalDecision(experiment);
    break;
  }

  return *this;
}

/// It'll add the experiment and submission to the pool, regardless of the
/// significance.
///
/// It'll update the verdict on whether the researcher will continue hacking or
/// not. The researcher can check this flag by calling `isStillHacking()`
/// routine.
///
/// @param experiment A reference to the experiment
void PatientDecisionMaker::initDecision(Experiment &experiment) {

  experiments_pool.push_back(experiment);
  submissions_pool.push_back(current_submission);

  is_still_hacking = true;
}

/// A patient decision maker is still optimizing for the effort, he'd not
/// continue hacking if an intermediate result is already publishable
///
/// @param experiment A reference to the experiment
void PatientDecisionMaker::intermediateDecision(Experiment &experiment) {

  is_still_hacking = !willBeSubmitting();
}

/// Patient decision maker keeps track of its intermediate results until it
/// makes the final decision and choose between them.
///
/// @param experiment A reference to the experiment
void PatientDecisionMaker::afterhackDecision(Experiment &experiment) {

  if (willBeSubmitting()) {
    experiments_pool.push_back(experiment);
    submissions_pool.push_back(current_submission);
  }

  // Patient Decision maker always continue hacking...
  is_still_hacking = true;
}

/// Impatient decision maker — at the final stage — goes through all solutions
/// and select the one based on its preference.
///
/// @param experiment A reference to the experiment.
void PatientDecisionMaker::finalDecision(Experiment &experiment) {

  final_submission = selectBetweenSubmissions();

  clearHistory();

  is_still_hacking = false;
}

PatientDecisionMaker &PatientDecisionMaker::verdict(Experiment &experiment,
                                                    DecisionStage stage) {

  current_submission = selectOutcome(experiment);

  switch (stage) {

  case DecisionStage::Initial:
    initDecision(experiment);
    break;
  case DecisionStage::WhileHacking:
    intermediateDecision(experiment);
    break;
  case DecisionStage::DoneHacking:
    afterhackDecision(experiment);
    break;
  case DecisionStage::Final:
    finalDecision(experiment);
    break;
  }

  return *this;
}

HonestDecisionMaker &HonestDecisionMaker::verdict(Experiment &experiment,
                                                  DecisionStage stage) {

  current_submission = selectOutcome(experiment);

  submissions_pool.push_back(current_submission);
  experiments_pool.push_back(experiment);

  final_submission = current_submission;

  return *this;
}
