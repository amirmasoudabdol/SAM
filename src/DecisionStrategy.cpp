//
// Created by Amir Masoud Abdol on 2019-02-01.
//

#include "DecisionStrategy.h"

using namespace sam;

///
/// Pure deconstructor of DecisionStrategy. This is necessary for proper
/// deconstruction of derived classes.
///
DecisionStrategy::~DecisionStrategy(){
    // pure deconstructor
};

DecisionStrategy::DecisionStrategy() {

  // Preparing a Lua instance, and registering my types there

  lua.open_libraries();

  lua.new_usertype<Group>("GroupData", "id", &Group::id_, "nobs",
                              &Group::nobs_, "pvalue", &Group::pvalue_,
                              "effect", &Group::effect_, "sig",
                              &Group::sig_);

  lua.new_usertype<Submission>(
      "Submission", "id",
      sol::property([](Submission &s) { return s.group_.id_; }), "nobs",
      sol::property([](Submission &s) { return s.group_.nobs_; }), "mean",
      sol::property([](Submission &s) { return s.group_.mean_; }), "pvalue",
      sol::property([](Submission &s) { return s.group_.pvalue_; }), "effect",
      sol::property([](Submission &s) { return s.group_.effect_; }), "sig",
      sol::property([](Submission &s) { return s.group_.sig_; }));
};

std::unique_ptr<DecisionStrategy>
DecisionStrategy::build(json &decision_strategy_config) {

  if (decision_strategy_config["name"] == "MarjansDecisionMaker") {

    auto params =
        decision_strategy_config.get<DefaultDecisionMaker::Parameters>();
    return std::make_unique<DefaultDecisionMaker>(params);

  } else {
    throw std::invalid_argument("Unknown DecisionStrategy");
  }
}



/// Select an unique outcome from an experiment, if at some point, a PolicyChain
/// finds a group of outcomes instread of a unique outcome, the selection will be
/// saved and the will await for processing in a different stages.
///
/// We check all available PolicyChains in the given chain set, and will stop as soon
/// as any chain returns __something__!
///
/// @param experiment a reference to an experiment
/// @param pchain_set a reference to a policy chain set
void DecisionStrategy::selectOutcome(Experiment &experiment,
                                     PolicyChainSet &pchain_set) {

//  spdlog::debug("---");
  assert(!pchain_set.empty() && "PolicyChainSet is empty!");
  
  for (auto &pchain : pchain_set) {
    
    auto selections = pchain(experiment);
    
    if (selections) {
      if (selections.value().size() == 1) {
        submission_candidate = selections.value().back();
        return;
      } else {
        /// If we find many, we just collect them, and quit
        submissions_pool.insert(submissions_pool.end(), selections.value().begin(), selections.value().end());
        return;
      }
    }
  }

}


/// Select a unique submission from the given pool of submissions. If none of the submissions
/// satisfies all the policies, we just return, and submission_candidate will not be rewritten.
///
/// @note If submission_candidate is empty to this point, and we cannot find anything here,
/// we'll continue with nothing, and the current experiment will most likely be discarded.
///
/// @param spool a collection of submissions collected in previous stages, e.g., selectOutcome
/// @param pchain_set a set of policy chains
void DecisionStrategy::selectBetweenSubmissions(SubmissionPool &spool,
                                                PolicyChainSet &pchain_set) {
  
  spdlog::debug("→ Current Submission Pool...");
  for (auto &s : spool) {
    spdlog::debug("\t\t{}", s);
  }
  
  /// @todo Check what this actually means!
  if (pchain_set.empty())
    return;   // we just don't have anything to work with

  for (auto &pchain : pchain_set) {
    
    auto selection = pchain(spool);
    
    if (selection) {
      submission_candidate = selection.value();
      return;
    } else {
      /// We didn't find anything
      return;
    }
    
  }


  spdlog::debug("✗ Found none in the pile!");
}


  /// Create and save all possible submissions from an experiment, if 
  /// the satisfy all of the given policies in the pchain.
  ///
  /// @param      experiment  a reference to the experiment
  /// @param      pchain      a policy chain, usually stored in
  ///                         `stashing_policy` in the config file
 void DecisionStrategy::saveOutcomes(Experiment &experiment, PolicyChain &pchain) {
    
    spdlog::debug("Stashing...");
    
    auto selections = pchain(experiment);
    
    if (selections) {
      submissions_pool.insert(submissions_pool.end(), selections.value().begin(),
                              selections.value().end());
    }
    
  }

bool DecisionStrategy::willBeSubmitting(const std::optional<Submission>& sub, PolicyChain &pchain) {

  if (pchain.empty() and sub)
    return true;
  else
    return false;
  
  // Checking whether all policies are returning `true`
  if (sub)
      return std::all_of(pchain.begin(), pchain.end(), [&](auto &policy) {
        return policy.func(sub.value());
      });
  else
      return false;

}


/// @brief  Decides whether we are going to start hacking or not.
/// In this canse, we only check if the `current_submission` complies with
/// `will_start_hacking_decision_policies` roles; if yes, we will start hacking
/// if no, then we will not continue to the hacking procedure
bool DefaultDecisionMaker::willStartHacking() {
    if (will_start_hacking_decision_policies.empty())
      return true;
    
    if (submission_candidate) {
      return std::any_of(will_start_hacking_decision_policies.begin(), will_start_hacking_decision_policies.end(), [this](auto &policy) -> bool {
          return policy.func(this->submission_candidate.value());
          });
    }else{
      return true;
    }
};


/// Determines whether the `final_submission_candidates` complies with **any** of the
/// given policies.
///
/// @note The important difference between this and `willBeSubmitting` is the fact that, the latter
/// will check if **all** of the rules are passing.
///
/// @todo This probably needs to be replaced by something inside the PolicyChain
///
/// @param pchain a reference to the given policy chain
bool DefaultDecisionMaker::willContinueHacking(Experiment *experiment,
                                               PolicyChain &pchain) {
  
  // Checking whether all policies are returning `true`
  
  if (pchain.empty())
    return true;
  
  bool verdict {true};
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
  ++i, ++d %= experiment->setup.nd()) {
    verdict &= std::any_of(pchain.begin(), pchain.end(), [&](auto &policy) -> bool {
                  return policy.func(Submission{*experiment, i});
                });
  }
  
  return verdict;
  
};


bool DefaultDecisionMaker::willContinueReplicating(PolicyChain &pchain) {
  
  // Checking whether all policies are returning `true`
  
  if (pchain.empty())
    return true;
  
  if (submission_candidate) {
    return std::any_of(pchain.begin(), pchain.end(), [this](auto &policy) -> bool {
        return policy.func(this->submission_candidate.value());
        });
  }else{
    return true;
  }
  
};

DecisionStrategy &DefaultDecisionMaker::selectOutcomeFromExperiment(Experiment *experiment,
                                                PolicyChainSet &pchain_set) {
  selectOutcome(*experiment, pchain_set);
  
  saveOutcomes(*experiment, stashing_policy);
  
  spdlog::debug("Collected pile of every submission: ");
  for (auto &s : submissions_pool) {
    spdlog::debug("\t{}", s);
  }
  return *this;
}
  
DecisionStrategy &DefaultDecisionMaker::selectOutcomeFromPool(SubmissionPool &spool,
                                                PolicyChainSet &pchain_set) {
  selectBetweenSubmissions(spool, pchain_set);
  clear();
  return *this;
}
