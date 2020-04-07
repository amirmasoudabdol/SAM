//
// Created by Amir Masoud Abdol on 2019-02-01.
//

#include <fmt/core.h>
#include <fmt/format.h>
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>

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

  if (decision_strategy_config["_name"] == "ImpatientDecisionMaker") {

    auto params =
        decision_strategy_config.get<ImpatientDecisionMaker::Parameters>();
    return std::make_unique<ImpatientDecisionMaker>(params);

  } else if (decision_strategy_config["_name"] == "PatientDecisionMaker") {

    auto params =
        decision_strategy_config.get<PatientDecisionMaker::Parameters>();
    return std::make_unique<PatientDecisionMaker>(params);

  } else if (decision_strategy_config["_name"] == "MarjansDecisionMaker") {

    auto params =
        decision_strategy_config.get<MarjansDecisionMaker::Parameters>();
    return std::make_unique<MarjansDecisionMaker>(params);

  } else {
    throw std::invalid_argument("Unknown DecisionStrategy");
  }
}

template <typename ForwardIt>
std::tuple<bool, ForwardIt, ForwardIt>
DecisionStrategy::checkThePolicy(const ForwardIt &begin, ForwardIt &end,
                                 Policy &p) {

  switch (p.type) {

  case PolicyType::Min: {
    auto it = std::min_element(begin, end, p.func);
    spdlog::debug("Min: {}", p.def);
    spdlog::debug("\t {}", *it);
    return {true, it, it};
  } break;

  case PolicyType::Max: {
    auto it = std::max_element(begin, end, p.func);
    spdlog::debug("Max: {}", p.def);
    spdlog::debug("\t {}", *it);
    return {true, it, it};
  } break;

  case PolicyType::Comp: {
    auto pit = std::partition(begin, end, p.func);
    spdlog::debug("Comp: {}", p.def);
    for (auto it{begin}; it != pit; ++it) {
      spdlog::debug("\t {}", *it);
    }

    return {false, begin, pit};

  } break;

  case PolicyType::Random: {
    /// Shuffling the array and setting the end pointer to the first time,
    /// this basically mimic the process of selecting a random element from
    /// the list.
    Random::shuffle(begin, end);
    spdlog::debug("Shuffled: {}", p.def);
    for (auto it{begin}; it != end; ++it) {
      spdlog::debug("\t {}", *it);
    }
    return {true, begin, end};

  } break;

  case PolicyType::First: {

    // Sorting the groups based on their index
    std::sort(begin, end, p.func);

    spdlog::debug("First: {}", p.def);
    spdlog::debug("\t {}", *begin);

    return {true, begin, end};

  } break;

  case PolicyType::All: {
    return {false, begin, end};

  } break;
  }
}

void DecisionStrategy::selectOutcome(Experiment &experiment,
                                     PolicyChainSet &pchain_set) {

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

      /// Basically we will not find anything anymore.
      /// This is not for performance reason, in some cases, e.g., "first",
      /// will behave incorrectly if we continue looking.
      if (begin == end)
        break;

      if (found_sth_unique) {
        current_submission_candidate = Submission{experiment, begin->id_};
        final_submission_candidate = current_submission_candidate;
        has_any_candidates = true;
        has_a_final_candidate = true;
        return;
      }
    }

    if (begin != end and not found_sth_unique) { /// We found a bunch

      spdlog::debug("Selected Bunch: ");
      for (auto it{begin}; it != end; ++it) {
        submissions_pool.emplace_back(experiment, it->id_);
        spdlog::debug("\t {}", *it);
      }
      has_any_candidates = true;
      return;
    } else {
      spdlog::debug("> Going to the next set of policies.");
    }

    pset_inx++;
  }

  has_any_candidates = false;
}

void DecisionStrategy::selectBetweenSubmissions(SubmissionPool &spool,
                                                PolicyChainSet &pchain_set) {

  spdlog::debug("Selecting between collected submissions.");
  
  assert(!pchain_set.empty() && "PolicySet is empty!");

  for (auto &pchain : pchain_set) {

    auto found_something{false};
    auto begin = spool.begin();
    auto end = spool.end();

    for (auto &policy : pchain) {

      std::tie(found_something, begin, end) =
          checkThePolicy(begin, end, policy);

      if (found_something) {
        spdlog::debug("> Found something in the pile!");
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

  spdlog::debug("Not happy!");
}

bool DecisionStrategy::willBeSubmitting(PolicyChain &pset) {

  // Checking whether all policies are returning `true`
  auto is_it_submittable =
      std::all_of(pset.begin(), pset.end(), [this](auto &policy) {
        return policy.func(this->final_submission_candidate);
      });

  return is_it_submittable;
}

DecisionStrategy &ImpatientDecisionMaker::verdict(Experiment *experiment,
                                                  PolicyChainSet &pchain_set) {
  selectOutcome(*experiment, pchain_set);
  return *this;
}

DecisionStrategy &ImpatientDecisionMaker::verdict(SubmissionPool &spool,
                                                  PolicyChainSet &pchain_set) {
  clearHistory();
  return *this;
}

DecisionStrategy &PatientDecisionMaker::verdict(Experiment *experiment,
                                                PolicyChainSet &pchain_set) {
  selectOutcome(*experiment, pchain_set);
  return *this;
}

DecisionStrategy &PatientDecisionMaker::verdict(SubmissionPool &spool,
                                                PolicyChainSet &pchain_set) {
  selectBetweenSubmissions(spool, pchain_set);
  clearHistory();
  return *this;
}

DecisionStrategy &MarjansDecisionMaker::verdict(Experiment *experiment,
                                                PolicyChainSet &pchain_set) {
  selectOutcome(*experiment, pchain_set);
  saveEverySubmission(*experiment);
  return *this;
}

DecisionStrategy &MarjansDecisionMaker::verdict(SubmissionPool &spool,
                                                PolicyChainSet &pchain_set) {
  selectBetweenSubmissions(spool, pchain_set);
  clearHistory();
  return *this;
}
