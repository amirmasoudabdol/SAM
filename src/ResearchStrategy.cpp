//===-- ResearchStrategy.h - Decision Strategy Implementation -------------===//
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
/// Pure destructors of ResearchStrategy. This is necessary for proper
/// deconstruction of derived classes.
///
ResearchStrategy::~ResearchStrategy() {
  // pure destructors
}

///
/// Besides constructing the base class, it also registers the Submission and
/// DependentVariable in Lua for later use of the derived classes, e.g.,
/// DefaultResearchStrategy
///
ResearchStrategy::ResearchStrategy() {
  // Preparing a Lua instance, and registering my types there
  lua.open_libraries();

  // Registering the DependentVariable
  lua.new_usertype<DependentVariable>(
      "DependentVariable", "id", &DependentVariable::id_, "nobs",
      &DependentVariable::nobs_, "mean", &DependentVariable::mean_, "pvalue",
      &DependentVariable::pvalue_, "effect", &DependentVariable::effect_, "sig",
      &DependentVariable::sig_, "hacked", &DependentVariable::is_hacked_,
      "candidate", &DependentVariable::is_candidate_);

  // Registering the Submission
  lua.new_usertype<Submission>(
      "Submission", "id",
      sol::property([](Submission &s) { return s.dv_.id_; }), "nobs",
      sol::property([](Submission &s) { return s.dv_.nobs_; }), "mean",
      sol::property([](Submission &s) { return s.dv_.mean_; }), "pvalue",
      sol::property([](Submission &s) { return s.dv_.pvalue_; }), "effect",
      sol::property([](Submission &s) { return s.dv_.effect_; }), "sig",
      sol::property([](Submission &s) { return s.dv_.sig_; }), "hacked",
      sol::property([](Submission &s) { return s.dv_.is_hacked_; }),
      "candidate",
      sol::property([](Submission &s) { return s.dv_.is_candidate_; }));
}

std::unique_ptr<ResearchStrategy> ResearchStrategy::build(
    json &research_strategy_config) {
  if (research_strategy_config["name"] == "DefaultResearchStrategy") {
    auto params =
        research_strategy_config.get<DefaultResearchStrategy::Parameters>();
    return std::make_unique<DefaultResearchStrategy>(params);

  } 

  throw std::invalid_argument("Unknown ResearchStrategy");
}

///
/// Select an unique outcome from an experiment, if at some point, a PolicyChain
/// finds a group of outcomes instead of a unique outcome, the selection will be
/// saved and the will await for processing in a different stages.
///
/// We check all available PolicyChains in the given chain set, and will stop as
/// soon as any chain returns __something__!
///
/// @param experiment a reference to an experiment
/// @param pchain_set a reference to a policy chain set
///
std::optional<SubmissionPool> ResearchStrategy::selectOutcome(Experiment &experiment,
                                     PolicyChainSet &pchain_set) {
  for (auto &pchain : pchain_set) {
    submission_candidates = pchain(experiment);

    /// If any of the pchains return something, we ignore the rest, and leave!
    if (submission_candidates) {
      return submission_candidates;
    }
  }
  
  return std::nullopt;
}

/// Select a unique submission from the given pool of submissions. If none of
/// the submissions satisfies all the policies, we just return, and
/// submission_candidate will not be rewritten.
///
/// @note If submission_candidate is empty to this point, and we cannot find
/// anything here, we'll continue with nothing, and the current experiment will
/// most likely be discarded.
///
/// @param spool a collection of submissions collected in previous stages, e.g.,
/// selectOutcome
/// @param pchain_set a set of policy chains
std::optional<SubmissionPool> ResearchStrategy::selectBetweenSubmissions(SubmissionPool &spool,
                                                PolicyChainSet &pchain_set) {
  /// If there is no policy defined, then, we don't do any filtering and returns
  /// all of the candidates
  if (pchain_set.empty()) {
    return spool;
    // return;  // we just don't have anything to work with
  }

  for (auto &pchain : pchain_set) {
    submission_candidates = pchain(spool);

    /// If any of the pchains return something, we ignore the rest, and leave!
    if (submission_candidates) {
      return submission_candidates;
    }
  }
  
  return std::nullopt;
}

/// Create and save all possible submissions from an experiment, if
/// the satisfy all of the given policies in the pchain.
///
/// @param      experiment  a reference to the experiment
/// @param      pchain      a policy chain, usually stored in
///                         `stashing_policy` in the config file
void ResearchStrategy::saveOutcomes(Experiment &experiment,
                                    PolicyChain &pchain) {
  if (not pchain.empty()) {
    spdlog::debug("Stashing...");

    auto selections = pchain(experiment);

    if (selections) {
      stashed_submissions.insert(stashed_submissions.end(),
                                 selections.value().begin(),
                                 selections.value().end());
    }
  }
}

///
/// @brief      Indicates whether the Researcher is going to submit the given
/// set of submissions or not.
///
/// Usually `submission_decision_policies` will be used to evaluate the quality
/// of the submission pool
///
/// @param[in]  subs    A list of Submission(s)
/// @param      pchain  A policy chain
///
/// @return     returns `true` if submission will undergo
///
bool ResearchStrategy::willBeSubmitting(
    const std::optional<std::vector<Submission>> &subs, PolicyChain &pchain) {
  
  if (pchain.empty()) {
    return true;
  }
  
    // Checking whether all policies are returning `true`
  if (subs) {
    bool check{false};
    for (const auto &sub : subs.value()) {
      check |= std::all_of(pchain.begin(), pchain.end(),
                           [&](auto &policy) -> bool { return policy(sub); });
    }
    return check;
  } 
  
  return false;
}

/// @brief  Decides whether Researcher is going to hacking the Experiment
///
/// In this case, we only check if the `current_submission` complies with
/// `will_start_hacking_decision_policies` roles; if yes, we will start hacking
/// if no, then we will not continue to the hacking procedure
bool DefaultResearchStrategy::willStartHacking(std::optional<SubmissionPool> &subs) {
  spdlog::trace("Checking whether to start hacking or not...");

  /// Start hacking if there is no criteria is defined
  if (will_start_hacking_decision_policies.empty()) {return true;}

  if (subs) {
    spdlog::trace("Looking for: {}", will_start_hacking_decision_policies);
    spdlog::trace("Submission Candidates: {}", subs.value());

    /// Basically any of the candidates is good enough, then we're going
    /// to STOP hacking
    bool verdict{false};
    /// @todo this can be replaced by Policy->operator()
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

/// Determines whether the `final_submission_candidates` complies with **any**
/// of the given policies.
///
/// @note The important difference between this and `willBeSubmitting` is the
/// fact that, the latter will check if **all** of the rules are passing.
///
/// @todo This probably needs to be replaced by something inside the PolicyChain
///
/// @param pchain a reference to the given policy chain
bool DefaultResearchStrategy::willContinueHacking(Experiment *experiment,
                                                  PolicyChain &pchain) {
  // Checking whether all policies are returning `true`

  if (pchain.empty()) { return true; }

  bool verdict{true};
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {
    verdict &=
        std::any_of(pchain.begin(), pchain.end(), [&](auto &policy) -> bool {
          return policy(Submission{*experiment, i});
        });
  }

  return verdict;
}

bool DefaultResearchStrategy::willContinueReplicating(SubmissionPool &subs) {
  // Checking whether all policies are returning `true`

  if (will_continue_replicating_decision_policy.empty()) {return true;}

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

std::optional<SubmissionPool> DefaultResearchStrategy::selectOutcomeFromExperiment(
   Experiment *experiment, PolicyChainSet &pchain_set) {
 
 /// @todo Check if you can implement this a bit nicer
 saveOutcomes(*experiment, stashing_policy);

 return selectOutcome(*experiment, pchain_set);
}

std::optional<SubmissionPool> DefaultResearchStrategy::selectOutcomeFromPool(
   SubmissionPool &spool, PolicyChainSet &pchain_set) {
 return selectBetweenSubmissions(spool, pchain_set);

 // return *this;
}
