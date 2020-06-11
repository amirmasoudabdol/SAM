//
// Created by Amir Masoud Abdol on 2019-02-01.
//

#include "DecisionStrategy.h"

namespace sam {
  bool isItSatisfactory(Experiment &experiment,
                              PolicyChain &pchain) {

    assert(!pchain.empty() && "PolicySet is empty!");
    spdlog::debug("Deciding whether we are going to continue hacking!");
    
    auto found_sth_unique{false};
    auto begin = experiment.groups_.begin() + experiment.setup.nd();
    auto end = experiment.groups_.end();

    for (auto &p : pchain) {
      std::tie(found_sth_unique, begin, end) = checkThePolicy(begin, end, p);

      if (found_sth_unique ) {
        return true;
      }
      
      if (begin == end){
        return false;
      }
      
    }
    
    // This has to be here
    if (begin+1 == end) {
      spdlog::debug("There is only one!");
      return true;
    }else if (begin != end) { /// We found a bunch

      return true;
    } else {
      return false;
    }


    return false;
  };
}


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

  lua.new_usertype<GroupData>("GroupData", "id", &GroupData::id_, "nobs",
                              &GroupData::nobs_, "pvalue", &GroupData::pvalue_,
                              "effect", &GroupData::effect_, "sig",
                              &GroupData::sig_);

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

  if (decision_strategy_config["_name"] == "MarjansDecisionMaker") {

    auto params =
        decision_strategy_config.get<MarjansDecisionMaker::Parameters>();
    return std::make_unique<MarjansDecisionMaker>(params);

  } else {
    throw std::invalid_argument("Unknown DecisionStrategy");
  }
}



void DecisionStrategy::selectOutcome(Experiment &experiment,
                                     PolicyChainSet &pchain_set) {

//  spdlog::debug("---");
  assert(!pchain_set.empty() && "PolicyChainSet is empty!");
  
  int pset_inx{0};
  for (auto &pchain : pchain_set) {

    /// These needs to be reset since I'm starting a new set of policies
    /// New policies will scan the set again!
    auto found_sth_unique{false};
    auto begin = experiment.groups_.begin() + experiment.setup.nd();
    auto end = experiment.groups_.end();

    for (auto &p : pchain) {
      std::tie(found_sth_unique, begin, end) = checkThePolicy(begin, end, p);

      if (found_sth_unique) {
        submission_candidate = Submission{experiment, begin->id_};
        spdlog::debug("✓ Found One!");
        return;
      }
      
      if (begin == end) {
        spdlog::debug("✗ Found nothing!");
        break;
      }
      
    }
    
    // This has to be here
    if (begin+1 == end) {
      submission_candidate = Submission{experiment, begin->id_};
      spdlog::debug("✓ Found the only one!");
      return;
    }else if (begin != end) { /// We found a bunch

      spdlog::debug("✓ Found a bunch: ");
      for (auto it{begin}; it != end; ++it) {
        submissions_pool.emplace_back(experiment, it->id_);
        spdlog::debug("\t {}", *it);
      }
      return;
    } else {
      spdlog::debug("✗ Found nothing! To the next one!");
    }

    pset_inx++;
  }

}

void DecisionStrategy::selectBetweenSubmissions(SubmissionPool &spool,
                                                PolicyChainSet &pchain_set) {
  
  spdlog::debug("→ Current Submission Pool...");
  for (auto &s : spool) {
    spdlog::debug("\t\t{}", s);
  }
  
  /// \todo Check what this actually means!
  if (pchain_set.empty())
    return;   // we just don't have anything to work with

  for (auto &pchain : pchain_set) {

    auto found_sth_unique{false};
    auto begin = spool.begin();
    auto end = spool.end();

    for (auto &policy : pchain) {

      std::tie(found_sth_unique, begin, end) =
          checkThePolicy(begin, end, policy);

      if (found_sth_unique) {
        spdlog::debug("✓ Found something in the pile!");
        submission_candidate = *begin;
        return;
      }
        
      if (begin == end)
          break;
        /// else:
        ///     We are still looking. This happens when I'm testing a comparison
    }
    
    if (begin+1 == end) {
      submission_candidate = *begin;
      spdlog::debug("✓ Found the only one!");
      return;
    } else if (begin != end) { /// We found a bunch
      /// This is not a acceptable case for now!
      /// \todo But it should be!
      return;
    } else {
      spdlog::debug("✗ Found nothing!");
    }
    
  }


  spdlog::debug("✗ Found none in the pile!");
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
bool MarjansDecisionMaker::willStartHacking() {
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
/// @param pchain a reference to the given policy chain
bool MarjansDecisionMaker::willContinueHacking(Experiment *experiment,
                                               PolicyChain &pchain) {
  
  // Checking whether all policies are returning `true`
  
  if (pchain.empty())
    return true;
  
//  if (submission_candidate) {
//    return std::any_of(pchain.begin(), pchain.end(), [this](auto &policy) -> bool {
//        return policy.func(this->submission_candidate.value());
//        });
//  }else{
//    return true;
//  }
  
  bool verdict {true};
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
  ++i, ++d %= experiment->setup.nd()) {
    verdict &= std::any_of(pchain.begin(), pchain.end(), [&](auto &policy) -> bool {
                  return policy.func(Submission{*experiment, i});
                });
  }
  
  return verdict;
  
};


bool MarjansDecisionMaker::willContinueReplicating(PolicyChain &pchain) {
  
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

DecisionStrategy &MarjansDecisionMaker::selectOutcomeFromExperiment(Experiment *experiment,
                                                PolicyChainSet &pchain_set) {
  selectOutcome(*experiment, pchain_set);
  
  saveOutcomes(*experiment, stashing_policy);
  
  spdlog::debug("Collected pile of every submission: ");
  for (auto &s : submissions_pool) {
    spdlog::debug("\t{}", s);
  }
  return *this;
}
  
DecisionStrategy &MarjansDecisionMaker::selectOutcomeFromPool(SubmissionPool &spool,
                                                PolicyChainSet &pchain_set) {
  selectBetweenSubmissions(spool, pchain_set);
  clear();
  return *this;
}
