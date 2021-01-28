//===-- RSDefaultResearchStrategy.cpp - Default Research Strategy Imp. ----===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2021-01-27.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This files contains the implementation of the Decision Strategy class which
/// is responsible for almost all the decision making throughout the research
/// process.
///
//===----------------------------------------------------------------------===//


#include "Policy.h"
#include "ResearchStrategy.h"

using namespace sam;

///
/// In this case, researcher checks if the list of current submissions complies
/// with `will_not_start_hacking_decision_policies`; if so, it will NOT start
/// hacking; otherwise, it WILL start the hacking procedure, and proceed to
/// either stashing or replicating.
///
/// @note       Basically, if _at least_ one of the submissions satisfy all of
///             the criteria, the researcher will not commit to the hacking.
///
/// @param      subs  A list of submission candidates
///
/// @return     Returns true if the researcher has to proceed with the hacking
///             strategies.
///
bool DefaultResearchStrategy::willStartHacking(
    std::optional<SubmissionPool> &subs) {
  spdlog::trace("Checking whether to start hacking or not...");

  // Start hacking if there is no criteria is defined
  if (will_not_start_hacking_decision_policies.empty()) {
    return true;
  }

  if (subs) {
    spdlog::trace("Will NOT start hacking if: {}", will_not_start_hacking_decision_policies);
    spdlog::trace("Submission Candidates: {}", subs.value());

    // Checking if any of the subs satisfying any of the policies, if so, then,
    // we have to continue hacking...
    return std::none_of(subs.value().begin(), subs.value().end(),
                       [&](auto &s) -> bool {
      return std::all_of(will_not_start_hacking_decision_policies.begin(),
                         will_not_start_hacking_decision_policies.end(),
                         [&](auto &policy) -> bool { return policy(s); });
    });
  }

  spdlog::trace("No Candidate is available → Will Start Hacking");
  return true;
}


/// Similar to the willStartHacking() checks whether any of the dependent 
/// variables are satisfying all `pchain` policies.
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

  spdlog::debug("Will NOT continue hacking if: {}", pchain);

  return std::none_of(experiment->dvs_.begin() + experiment->setup.nd(),
                      experiment->dvs_.end(), [&](auto &dv) {
                        return std::all_of(
                            pchain.begin(), pchain.end(),
                            [&](auto &policy) { return policy(dv); });
                      });
}

///
/// Overload of the willContinueHacking() that operates on a list of submissions
/// instead of an experiment.
///
/// @param      subs    The list of submissions, if any
/// @param      pchain  The policy chain
///
/// @return     Retruns `true` if the researcher should proceed with the next
///             hacking strategy
///
bool DefaultResearchStrategy::willContinueHacking(std::optional<SubmissionPool> &subs,
              PolicyChain &pchain) {

  if (pchain.empty()) {
    return true;
  }

  spdlog::debug("Will NOT continue hacking if: {}", pchain);

  if (subs) {
    return std::none_of(
        subs.value().begin(), subs.value().end(), [&](auto &s) -> bool {
          return std::all_of(pchain.begin(), pchain.end(),
                             [&](auto &policy) -> bool { return policy(s); });
        });
  }

  return true;

}


///
/// Similar to the willContinueHacking(), but uses 
/// `will_not_continue_replicating_decision_policy` instead.
///
/// @param      subs  A list of submission candidates
///
/// @return     Retruns true if the researcher has to continue the replication
///             procedure
///
bool DefaultResearchStrategy::willContinueReplicating(SubmissionPool &subs) {
  
  spdlog::trace("Checking whether to continue replicating or not...");

  if (will_not_continue_replicating_decision_policy.empty()) {
    return true;
  }

  if (not subs.empty()) {
    spdlog::trace("Will NOT continue replicating if: {}",
                  will_not_continue_replicating_decision_policy);
    spdlog::trace("Submission Candidates: {}", subs);

    return std::none_of(subs.begin(), subs.end(), [&](auto &s) -> bool {
      return std::all_of(will_not_continue_replicating_decision_policy.begin(),
                         will_not_continue_replicating_decision_policy.end(),
                         [&](auto &policy) -> bool { return policy(s); });
    });
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
