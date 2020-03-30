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
                              &GroupData::nobs_, "pvalue",
                              &GroupData::pvalue_, "effect",
                              &GroupData::effect_, "sig", &GroupData::sig_);

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

  }
  else
  {
    throw std::invalid_argument("Unknown DecisionStrategy");
  }
}


template <typename ForwardIt>
std::tuple<bool, ForwardIt, ForwardIt>
DecisionStrategy::checkThePolicy(const ForwardIt &begin, ForwardIt &end, Policy &policy) {

  auto type = policy.first;
  auto func = policy.second;

  switch (type) {

  case PolicyType::Min: {
    auto it = std::min_element(begin, end, func);
    spdlog::debug("Min:"); spdlog::debug("\t {}", *it);
    return {true, it, it};
  } break;

  case PolicyType::Max: {
    auto it = std::max_element(begin, end, func);
    spdlog::debug("Max:"); spdlog::debug("\t {}", *it);
    return {true, it, it};
  } break;

  case PolicyType::Comp: {
    auto pit = std::partition(begin, end, func);
    spdlog::debug("Comp: ");
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
    spdlog::debug("Shuffled: ");
    for (auto it{begin}; it != end; ++it) {
      spdlog::debug("\t {}", *it);
    }
    return {true, begin, end};

  } break;

  case PolicyType::First: {

    // Sorting the groups based on their index
    std::sort(begin, end, func);

    spdlog::debug("First: "); spdlog::debug("\t {}", *begin);

    return {true, begin, end};

  } break;
      
  case PolicyType::All: {
    return {true, begin, end};
    
  } break;
      
  }
}

std::optional<Policy>
DecisionStrategy::registerPolicy(const std::string &s) {

  std::map<std::string, std::string> lua_temp_scripts{
      {"min_script", "function {} (l, r) return l.{} < r.{} end"},

      {"max_script", "function {} (l, r) return l.{} > r.{} end"},

      {"comp_script", "function {} (d) return d.{} end"}};

  std::map<std::string, std::string> cops = {
      {">=", "greater_eq"}, {"<=", "lesser_eq"}, {">", "greater"},
      {"<", "lesser"},      {"==", "equal"},     {"!=", "not_equal"}};

  PolicyType policy_type;
  sol::function policy_func;

  if (s.find("min") != std::string::npos) {

    auto open_par = s.find("(");
    auto close_par = s.find(")");

    auto var_name = s.substr(open_par + 1, close_par - open_par - 1);

    auto f_name = fmt::format("min_{}", var_name);
    auto f_def = fmt::format(lua_temp_scripts["min_script"], f_name, var_name,
                             var_name);

    lua.script(f_def);

    policy_type = PolicyType::Min;
    policy_func = lua[f_name];

  } else if (s.find("max") != std::string::npos) {

    auto open_par = s.find("(");
    auto close_par = s.find(")");

    auto var_name = s.substr(open_par + 1, close_par - open_par - 1);

    auto f_name = fmt::format("max_{}", var_name);
    auto f_def = fmt::format(lua_temp_scripts["max_script"], f_name, var_name,
                             var_name);

    lua.script(f_def);

    policy_type = PolicyType::Max;
    policy_func = lua[f_name];
  } else if (s.find("sig") != std::string::npos) {

    auto f_name = "cond_sig";

    auto f_def =
        fmt::format(lua_temp_scripts["comp_script"], f_name, "sig == true");

    lua.script(f_def);

    policy_type = PolicyType::Comp;
    policy_func = lua[f_name];

  } else if (s.find("random") != std::string::npos) {

    policy_type = PolicyType::Random;
    policy_func = sol::function();

  } else if (s.find("first") != std::string::npos) {

    auto var_name = "id";
    auto f_name = fmt::format("min_{}", var_name);
    auto f_def = fmt::format(lua_temp_scripts["min_script"], f_name, var_name,
                             var_name);

    lua.script(f_def);

    policy_type = PolicyType::First;
    policy_func = lua[f_name];
    
  }else if (s.find("all") != std::string::npos)  {
    
    auto var_name = "id";
    auto f_name = fmt::format("all_{}", var_name);
    auto f_def = fmt::format("function all_id () return true end");

    lua.script(f_def);

    policy_type = PolicyType::All;
    policy_func = lua[f_name];
   
  } else if (std::any_of(cops.begin(), cops.end(), [&s](const auto &op) {
               return s.find(op.first) != std::string::npos;
             })) {
    // Found a comparision

    std::string s_op{};
    for (const auto &op : cops)
      if (s.find(op.first) != std::string::npos) {
        s_op = op.first;
        break;
      }

    auto op_start = s.find(s_op);

    auto var_name = s.substr(0, op_start - 1);
    auto f_name = fmt::format("cond_{}", var_name + "_" + cops[s_op]);
    auto f_def = fmt::format(lua_temp_scripts["comp_script"], f_name,
                             s); // Full text goes here

    lua.script(f_def);

    policy_type = PolicyType::Comp;
    policy_func = lua[f_name];

  } else {
    // TODO: I need a throw here
    return {};
  }

  return std::make_pair(policy_type, policy_func);
}

PolicySet
DecisionStrategy::registerPolicySet(const std::vector<std::string> &policy_set_defs) {
  
  PolicySet policy_set;
  
  for (auto &s : policy_set_defs) {
    
    auto policy = registerPolicy(s);

    if (policy)
      policy_set.push_back(policy.value());
    
  }
  
  return policy_set;
}

PolicyChain
DecisionStrategy::registerPolicyChain(const std::vector<std::vector<std::string>> &policy_chain_def) {

  PolicyChain policy_chain;

  for (auto &policies : policy_chain_def) {

    policy_chain.push_back(PolicySet());

    for (auto &s : policies) {

      auto policy = registerPolicy(s);

      if (policy)
        policy_chain.back().push_back(policy.value());
    }
    
  }

  return policy_chain;
  
}

void DecisionStrategy::selectOutcome(Experiment &experiment, PolicyChain &pchain) {

  /// We always start with the pre_registered_group, and if we find others
  /// results based on researchers' preference, then we replace it, and report
  /// that one.
  /// CHECK ME: I'm not sure if this is a good way of doing this...
//  int selectedOutcome{0};

  int pre_registered_group = experiment.setup.nd();

  int pset_inx{0};
  for (auto &pset : pchain) {

    /// These needs to be reset since I'm starting a new set of policies
    /// New policies will scan the set again!
    auto found_something{false};
    auto begin = experiment.groups_.begin() + experiment.setup.nd();
    auto end = experiment.groups_.end();

    for (auto &p : pset) {

      std::tie(found_something, begin, end) = checkThePolicy(begin, end, p);

//      if (found_something) {
//        /// If we have a hit, mainly after going to Min, Max, First, Random;
//        /// then, we are done!
//        spdlog::debug("> Found something!");
//        selectedOutcome = begin->id_;
//        spdlog::debug("Policy: {}", pset_inx);
//        current_submission_candidate = Submission{experiment, selectedOutcome};
//        has_a_candidate = true;
//
//
//
//      } else {
//        /// The range is empty! This only happens when Comp case cannot find
//        /// anything with the given comparison. Then, we break out the loop, and
//        /// move into the new set of policies
//        if (begin == end) {
//          spdlog::debug("> Going to the next set of policies.");
//          break; // Out of the for-loop, going to the next chain
//        }
//
//        /// else:    We are still looking!
//      }
    }
    
    if (begin !=  end) {
      /// We found something
      spdlog::debug("Selected: ");
      for (auto it{begin}; it != end; ++it) {
        submissions_pool.emplace_back(experiment, it->id_);
        spdlog::debug("\t {}", *it);
      }
      // TODO: Remove me, I'm temporary
      current_submission_candidate = submissions_pool.back();
      has_a_candidate = true;
    }else{
      spdlog::debug("> Going to the next set of policies.");
    }
    

    pset_inx++;
  }
  
  has_a_candidate = false;
}

void DecisionStrategy::selectBetweenSubmissions(SubmissionPool &spool, PolicyChain &pchain) {

  spdlog::debug("Selecting between collected submissions.");
  
  for (auto &pset : pchain) {

    auto found_something{false};
    auto begin = spool.begin();
    auto end = spool.end();

    for (auto &policy : pset) {

      std::tie(found_something, begin, end) = checkThePolicy(begin, end, policy);

      if (found_something) {
        spdlog::debug("> Found something in the pile!");
        final_submission_candidate = *begin;
        return;
      } else {
        if (begin == end) break;
        /// else:
        ///     We are still looking. This happens when I'm testing a comparison
      }
    }
  }

  spdlog::debug("Not happy!");
  has_a_candidate = false;    // FIXME: What am I doing here?
}

bool DecisionStrategy::willBeSubmitting(PolicySet &pset) {

  // Checking whether all policies are returning `true`
  auto is_it_submittable = std::all_of(
      pset.begin(), pset.end(),
      [this](auto &policy) { return policy.second(this->final_submission_candidate); });

  return is_it_submittable;
  
}

DecisionStrategy &ImpatientDecisionMaker::verdict(Experiment *experiment,
                                  PolicyChain &pchain) {
  selectOutcome(*experiment, pchain);
  return *this;
}

DecisionStrategy &ImpatientDecisionMaker::verdict(SubmissionPool &spool,
                                  PolicyChain &pchain) {
  return *this;
}


DecisionStrategy &PatientDecisionMaker::verdict(Experiment *experiment,
                                  PolicyChain &pchain) {
  selectOutcome(*experiment, pchain);
  saveCurrentSubmission();
  return *this;
}

DecisionStrategy &PatientDecisionMaker::verdict(SubmissionPool &spool,
                                  PolicyChain &pchain) {
  selectBetweenSubmissions(spool, pchain);
  clearHistory();
  return *this;
}
