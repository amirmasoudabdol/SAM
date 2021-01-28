//===-- RSDefaultResearchStrategy.cpp - Default Research Strategy Imp. ----===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-02-01.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This files contains the implementation of the Decision Strategy class which
/// is responsible for almost all the decision making throughout the research
/// process.
///
//===----------------------------------------------------------------------===//


#include "ResearchStrategy.h"

using namespace sam;

///
/// In this case, researcher only checks if the `current_submission` complies
/// with `will_start_hacking_decision_policies`; if so, it will start hacking if
/// not, then it will not continue to the hacking procedure, and proceed to
/// either stashing or replicating.
///
/// @param      subs  A list of submission candidates
///
/// @return     Returns true if the researcher has to proceed with the hacking
///             strategies.
///
bool DefaultResearchStrategy::willStartHacking(
    std::optional<SubmissionPool> &subs) {
  spdlog::trace("Checking whether to start hacking or not...");
  spdlog::trace("Looking for {}", will_start_hacking_decision_policies);

  // Start hacking if there is no criteria is defined
  if (will_start_hacking_decision_policies.empty()) {
    return true;
  }

  if (subs) {
    spdlog::trace("Looking for: {}", will_start_hacking_decision_policies);
    spdlog::trace("Submission Candidates: {}", subs.value());

    // Basically any of the candidates is good enough, then we're going
    // to STOP hacking
    bool verdict{false};
    
    auto ss = will_start_hacking_decision_policies(subs.value());
    if (ss)
      spdlog::trace("subs: {}", ss.value());

    for (auto &sub : subs.value()) {
      verdict |= std::any_of(will_start_hacking_decision_policies.begin(),
                             will_start_hacking_decision_policies.end(),
                             [&](auto &policy) -> bool { return policy(sub); });
    }
    return verdict;
  }

  spdlog::trace("No Candidate is available → Will Start Hacking");
  return true;
}

/// Determines whether any of the Dependent variables complies with **any** of
/// the given policies.
///
/// @note       The important difference between this and willBeSubmitting() is
///             the fact that, the latter will check if **all** of the rules are
///             passing.
///
/// @todo       This probably needs to be replaced by something inside the
///             PolicyChain
///
/// @param      pchain  a reference to the given policy chain
///
/// @return     Retruns `true` is the researcher should proceed with the next
///             hacking strategy
bool DefaultResearchStrategy::willContinueHacking(Experiment *experiment,
                                                  PolicyChain &pchain) {
  
  // Checking whether all policies are returning `true`
  if (pchain.empty()) {
    return true;
  }

  bool verdict{true};
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {
    verdict &=
        std::any_of(pchain.begin(), pchain.end(), [&](auto &policy) -> bool {
          return policy(experiment->dvs_[i]);
        });
  }

  return verdict;
}

///
/// Similar to the willContinueHacking(), this returns true if .....
///
/// @param      subs  A list of submission candidates
///
/// @return     Retruns true if the researcher has to continue the replication
///             procedure
///
/// @todo       I need to rethink these
bool DefaultResearchStrategy::willContinueReplicating(SubmissionPool &subs) {
  // Checking whether all policies are returning `true`

  if (will_continue_replicating_decision_policy.empty()) {
    return true;
  }

  if (not subs.empty()) {
    bool verdict{false};
    for (auto &sub : subs) {
      verdict |= std::any_of(will_continue_replicating_decision_policy.begin(),
                             will_continue_replicating_decision_policy.end(),
                             [&](auto &policy) -> bool { return policy(sub); });
    }
    return not verdict;
  }

  return true;
}

///
/// If necessasry (ie., if stashing_policy is specified), it stashes some
/// submissions from the experiment and also it returns the result of applying
/// policy chain set on the experiment, if any.
///
/// @param      experiment  The experiment
/// @param      pchain_set  The policy chain set
///
/// @return     Returns a list of submissions, if any.
///
std::optional<SubmissionPool>
DefaultResearchStrategy::selectOutcomeFromExperiment(
    Experiment *experiment, PolicyChainSet &pchain_set) {
  /// @todo Check if you can implement this a bit nicer
  saveOutcomes(*experiment, stashing_policy);

  return selectOutcome(*experiment, pchain_set);
}

///
/// @param      spool       The submission pool
/// @param      pchain_set  The policy chain set
///
/// @return     Returns a list of submissions, if any.
///
std::optional<SubmissionPool> DefaultResearchStrategy::selectOutcomeFromPool(
    SubmissionPool &spool, PolicyChainSet &pchain_set) {
  return selectBetweenSubmissions(spool, pchain_set);
}
