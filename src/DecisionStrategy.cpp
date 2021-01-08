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

  if (decision_strategy_config["name"] == "DefaultDecisionMaker") {

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
  /// @todo I probably should check this somewhere else, and don't throw here!
//  assert(!pchain_set.empty() && "PolicyChainSet is empty!");
  
  for (auto &pchain : pchain_set) {
    
    auto selections = pchain(experiment);
    
    if (selections) {
      /// @todo optimize this by directly assigning pchain() output to selection
      submission_candidates = selections;
//      if (selections.value().size() == 1) {
//        submission_candidate = selections.value().back();
//      }
      /// This is the case where we managed to find more than one submission
      /// So, we return here
      return;
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
  
//  spdlog::debug("→ → Selecting from Submissions Pool...");
//
//  spdlog::trace("→ → Current Submission Pool: ");
//  for (auto &s : spool) {
//    spdlog::trace("\t\t{}", s);
//  }
  
  /// @todo Check what this actually means!
  if (pchain_set.empty()){
    submission_candidates = spool;
    return;   // we just don't have anything to work with
  }

  for (auto &pchain : pchain_set) {
    
    auto selection = pchain(spool);
    
    if (selection) {
      submission_candidates = selection;
//      if (selection.value().size() == 1){
//        submission_candidate = selection.value()[0];
//      }
      /// This is the case where we managed to find more than one submission
      /// So, we return here
      return;
    }

  }


  spdlog::trace("✗ Found none in the pile!");
}


/// Create and save all possible submissions from an experiment, if
/// the satisfy all of the given policies in the pchain.
///
/// @param      experiment  a reference to the experiment
/// @param      pchain      a policy chain, usually stored in
///                         `stashing_policy` in the config file
 void DecisionStrategy::saveOutcomes(Experiment &experiment, PolicyChain &pchain) {
    
   if (not pchain.empty()) {
     
    spdlog::debug("Stashing...");
   
    auto selections = pchain(experiment);
    
    if (selections) {
      stashed_submissions.insert(stashed_submissions.end(), selections.value().begin(),
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
bool DecisionStrategy::willBeSubmitting(const std::optional<std::vector<Submission>>& subs, PolicyChain &pchain) {
  
  if (pchain.empty() and subs)
    return true;
  else
    return false;
  
  // Checking whether all policies are returning `true`
  if (subs) {
    bool check {false};
    for (auto &sub : subs.value())
      check |= std::all_of(pchain.begin(), pchain.end(),
        [&](auto &policy) -> bool {
          return policy.func(sub);
        });
    return check;
  } else
    return false;
  
}


/// @brief  Decides whether Researcher is going to hacking the Experiment
///
/// In this case, we only check if the `current_submission` complies with
/// `will_start_hacking_decision_policies` roles; if yes, we will start hacking
/// if no, then we will not continue to the hacking procedure
bool DefaultDecisionMaker::willStartHacking() {
  
  spdlog::trace("Checking whether to start hacking or not...");

  /// Start hacking if there is no criteria is defined
  if (will_start_hacking_decision_policies.empty())
    return true;
    
  
  if (submission_candidates) {
    spdlog::trace("Looking for: {}", will_start_hacking_decision_policies);
    spdlog::trace("Submission Candidates: {}", submission_candidates.value());
    /// @todo this can be replaced by Policy->oprator()
    
    /// Basically any of the candidates is good enough, then we're going
    /// to STOP hacking
    bool verdict {false};
    for (auto &sub : submission_candidates.value())
      verdict |= std::any_of(will_start_hacking_decision_policies.begin(),
                             will_start_hacking_decision_policies.end(),
                             [&](auto &policy) -> bool {
                                return policy.func(sub);
                              });
    return verdict;
    
    
//    return std::any_of(will_start_hacking_decision_policies.begin(), will_start_hacking_decision_policies.end(), [this](auto &policy) -> bool {
//        return policy.func(this->submission_candidate.value());
//        });
  } else {
    spdlog::trace("No Candidate is available → Will Start Hacking");
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
  
  if (submission_candidates) {
    
    bool verdict {false};
    for (auto &sub : submission_candidates.value())
      verdict |= std::any_of(pchain.begin(),
                             pchain.end(),
                             [&](auto &policy) -> bool {
                                return policy.func(sub);
                              });
    return not verdict;
  }else{
    return true;
  }
  
};

DecisionStrategy &DefaultDecisionMaker::selectOutcomeFromExperiment(Experiment *experiment,
                                                PolicyChainSet &pchain_set) {
  selectOutcome(*experiment, pchain_set);
  
  /// @todo this is confusing and I need to change it
  saveOutcomes(*experiment, stashing_policy);
  
//  spdlog::trace("Collected pile of every hacked/submission: {}", stashed_submissions);
  
  return *this;
}

DecisionStrategy &DefaultDecisionMaker::selectOutcomeFromPool(SubmissionPool &spool,
                                                PolicyChainSet &pchain_set) {
  selectBetweenSubmissions(spool, pchain_set);
  
  /// @todo This looks terrible! A random clear() inside a function!
  /// @bug I need change this!
  clear();
  return *this;
}
