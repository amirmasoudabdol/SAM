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
  assert(!pchain_set.empty() && "PolicySet is empty!");
  
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
        current_submission_candidate = Submission{experiment, begin->id_};
        final_submission_candidate = current_submission_candidate;
        has_any_candidates = true;
        has_a_final_candidate = true;
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
      current_submission_candidate = Submission{experiment, begin->id_};
      final_submission_candidate = current_submission_candidate;
      has_any_candidates = true;
      has_a_final_candidate = true;
      spdlog::debug("✓ Found the only one!");
      return;
    }else if (begin != end) { /// We found a bunch

      spdlog::debug("✓ Found a bunch: ");
      for (auto it{begin}; it != end; ++it) {
        submissions_pool.emplace_back(experiment, it->id_);
        spdlog::debug("\t {}", *it);
      }
      has_any_candidates = true;
      return;
    } else {
      spdlog::debug("✗ Found nothing! To the next one!");
    }

    pset_inx++;
  }

  has_any_candidates = false;
}

void DecisionStrategy::selectBetweenSubmissions(SubmissionPool &spool,
                                                PolicyChainSet &pchain_set) {
  
  spdlog::debug("→ Current Submission Pool...");
  for (auto &s : spool) {
    spdlog::debug("\t\t{}", s);
  }
  
  assert(!pchain_set.empty() && "PolicySet is empty!");

  for (auto &pchain : pchain_set) {

    auto found_something{false};
    auto begin = spool.begin();
    auto end = spool.end();

    for (auto &policy : pchain) {

      std::tie(found_something, begin, end) =
          checkThePolicy(begin, end, policy);

      if (found_something) {
        spdlog::debug("✓ Found something in the pile!");
        final_submission_candidate = *begin;
        has_a_final_candidate = true;
        return;
      } else {
        if (begin == end)
          break;
        /// else:
        ///     We are still looking. This happens when I'm testing a comparison
      }
    }
  }

  spdlog::debug("✗ Found none in the pile!");
}

bool DecisionStrategy::willBeSubmitting(PolicyChain &pchain) {

  // Checking whether all policies are returning `true`
  auto is_it_submittable =
      std::all_of(pchain.begin(), pchain.end(), [this](auto &policy) {
        return policy.func(this->final_submission_candidate);
      });

  return is_it_submittable;
}

bool MarjansDecisionMaker::willStartHacking() {
  /// \todo: Watch out, this might cause issues, I'm calling it in other places with different signicutes
  return willContinueHacking(will_start_hacking_decision_policies);
};


/// Determines whether the `final_submission_candidates` complies with **any** of the
/// given policies.
///
/// @note The important difference between this and `willBeSubmitting` is the fact that, the latter
/// will check if **all** of the rules are passing.
///
/// @param pchain a reference to the given policy chain
bool MarjansDecisionMaker::willContinueHacking(PolicyChain &pchain) {
  
  // Checking whether all policies are returning `true`
  if (this->has_a_final_candidate) {
    return std::any_of(pchain.begin(), pchain.end(), [this](auto &policy) -> bool {
          return policy.func(this->final_submission_candidate);
        });
  }else{
    return true;
  }
  
};

DecisionStrategy &MarjansDecisionMaker::verdict(Experiment *experiment,
                                                PolicyChainSet &pchain_set) {
  selectOutcome(*experiment, pchain_set);
  saveEveryOutcome(*experiment);
  
  spdlog::debug("Collected pile of every submission: ");
  for (auto &s : submissions_pool) {
    spdlog::debug("\t{}", s);
  }
  return *this;
}
  
DecisionStrategy &MarjansDecisionMaker::verdict(SubmissionPool &spool,
                                                PolicyChainSet &pchain_set) {
  selectBetweenSubmissions(spool, pchain_set);
  clear();
  return *this;
}
