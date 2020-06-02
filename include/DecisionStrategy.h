//
// Created by Amir Masoud Abdol on 2019-02-01.
//

///
/// \defgroup   DecisionStrategies Decision Strategies
/// \brief      List of available Decision Strategies
///
/// Description to come!
///

#ifndef SAMPP_DECISIONSTRATEGY_H
#define SAMPP_DECISIONSTRATEGY_H

#include <fmt/core.h>
#include <fmt/format.h>
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>

#include "Experiment.h"
#include "Utilities.h"
#include "Policy.h"

using json = nlohmann::json;

namespace sam {

enum class DecisionMethod {
  MarjansDecisionMaker
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    DecisionMethod,
    {{DecisionMethod::MarjansDecisionMaker, "MarjansDecisionMaker"}})

using SubmissionPool = std::vector<Submission>;


template <typename ForwardIt>
std::tuple<bool, ForwardIt, ForwardIt>
checkThePolicy(const ForwardIt &begin, ForwardIt &end,
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
//    std::sort(begin, end, p.func);

    spdlog::debug("First: {}", p.def);
    spdlog::debug("\t {}", *begin);

    return {true, begin, end};

  } break;
      
  case PolicyType::Last: {

      // Sorting the groups based on their index
  //    std::sort(begin, end, p.func);

      spdlog::debug("Last: {}", p.def);
      spdlog::debug("\t {}", *begin);

      return {true, end, end};

  } break;

  case PolicyType::All: {
    return {false, begin, end};

  } break;
  }
}

bool isItSatisfactory(Experiment &experiment,
                      PolicyChain &pchain);

///
/// \brief      Abstract class for different decision strategies.
///
class DecisionStrategy {

public:
  // Lua state
  sol::state lua;

public:
  
  virtual ~DecisionStrategy() = 0;
  DecisionStrategy();
    
  json config_;
  
  /// DecisionStrategy factory method.
  ///
  /// \param decision_strategy_config    A JSON object containing information
  /// about each decision strategy.
  static std::unique_ptr<DecisionStrategy>
  build(json &decision_strategy_config);
  
  Submission current_submission_candidate;
  
  Submission final_submission_candidate;
  
  //! List of selected Submission by the researcher.
  SubmissionPool submissions_pool;

  /// TODO: These guys should move to their own class, I don't have to keep everything here!
  PolicyChainSet initial_selection_policies;
  PolicyChain submission_decision_policies;
  PolicyChainSet between_hacks_selection_policies;
  PolicyChainSet between_reps_policies;
  
  PolicyChain will_start_hacking_decision_policies;

  PolicyChain will_continue_replicating_decision_policy;


  //! If `true`, the Researcher will continue traversing through the
  //! hacknig methods, otherwise, he/she will stop the hacking and
  //! prepare the finalSubmission. It will be updated on each call of
  //! verdict(). Basically verdict() decides if the Researcher is
  //! happy with the submission record or not.
  bool is_still_hacking{false};

  bool will_be_submitting{false};

  bool has_any_candidates{false};
  
  bool has_a_final_candidate{false};

  bool hasSubmissionCandidate() const { return has_any_candidates; };

  /// Experiment
//  void operator()(Experiment *experiment, PolicyChainSet &pchain_set) {
//    selectOutcomeFromExperiment(experiment, pchain_set);
//  }
//
//  /// Submission Pool
//  void operator()(SubmissionPool &spool, PolicyChainSet &pchain_set) {
//    selectOutcomeFromPool(spool, pchain_set);
//  }
  
  /// Submission
  bool willBeSubmitting(PolicyChain &pchain);
  
  /// Clear the contents of the decision strategy, this include the
  /// submission pools or other collected information by the decision
  /// strategy.
  void clear() {
    submissions_pool.clear();
  }
  
  /// \todo: This needs to be private but currently, I don't have a good place to put it.
  /// The verdict system is broken, and if reset it after the selectionBetweenSubmission, it's werid
  /// and I cannot just call it in any other methods because then it's hidden
  /// 
  /// Reset the internal state of the decision strategy
  void reset() {
    is_still_hacking = false;
    will_be_submitting = false;
    has_any_candidates = false;
    has_a_final_candidate = false;
    clear();
  }
  
  
  /// The logic of continuation should be implemented here. Researcher will
  /// ask this method to asses the state of its progress.
  /// TODO: Consider making this just virtual and not pure abstract,
  /// maybe this implementation bool willBeHacking() override { return not has_any_candidates; };
//  virtual bool willBeHacking() {return not has_any_candidates; };
  
  virtual bool willStartHacking() { return not has_any_candidates; };
  
  
  
  /// @brief  This will be used by `letTheHackBegin` and uses the decision policy to decide
  /// whether the next hacking strategy is going to be executed or not!
  ///
  /// @param  experiment A reference to the experiment
  virtual bool willContinueHacking(PolicyChain &pchain) {return false;};
  
  
  /// \brief      Implementation of decision-making procedure.
  ///
  /// \param      experiment
  /// \param[in]  stage       The stage in which the researcher is asking
  ///                         for the verdict. The implementation of verdict
  ///                         sould provide different procedure for different
  ///                         stages of the development.
  ///
  /// \return     A boolean indicating whether result is satisfactory or not
  virtual DecisionStrategy &selectOutcomeFromExperiment(Experiment *experiment,
                                    PolicyChainSet &pchain_set) = 0;
  
  virtual DecisionStrategy &selectOutcomeFromPool(SubmissionPool &spool,
                                    PolicyChainSet &pchain_set) = 0;

  void saveEveryOutcome(Experiment &experiment) {
    for (int i{experiment.setup.nd()}, d{0}; i < experiment.setup.ng();
         ++i, ++d %= experiment.setup.nd()) {
      submissions_pool.emplace_back(experiment, i);
    }
  };
  
protected:
  
  /// A helper method to save the current submission. This needs to be called
  /// after verdict.
  void saveCurrentSubmission() {
    submissions_pool.push_back(current_submission_candidate);
  };
  

  
  void selectOutcome(Experiment &experiment, PolicyChainSet &pchain_set);

  void selectBetweenSubmissions(SubmissionPool &spool, PolicyChainSet &pchain_set);

  
};

///
/// \ingroup    DecisionStrategies
/// \brief      { item_description }
///
class MarjansDecisionMaker final : public DecisionStrategy {

public:
  struct Parameters {
    DecisionMethod name = DecisionMethod::MarjansDecisionMaker;

    std::vector<std::vector<std::string>> initial_selection_policies_defs;
    std::vector<std::string> submission_decision_policies_defs;
    std::vector<std::vector<std::string>> between_hacks_selection_policies_defs;
    std::vector<std::vector<std::string>> between_replications_selection_policies_defs;
    
    std::vector<std::string> will_start_hacking_decision_policies_def;
    
    std::vector<std::string> will_continue_replicating_decision_policy_def;
  };

  Parameters params;

  explicit MarjansDecisionMaker(const Parameters &p) : params{p} {

    initial_selection_policies = PolicyChainSet(p.initial_selection_policies_defs, lua);

    between_hacks_selection_policies = PolicyChainSet(p.between_hacks_selection_policies_defs, lua);

    submission_decision_policies = PolicyChain(p.submission_decision_policies_defs, lua);
    
    between_reps_policies = PolicyChainSet(p.between_replications_selection_policies_defs, lua);
    
    will_start_hacking_decision_policies = PolicyChain(p.will_start_hacking_decision_policies_def, lua);
    
    will_continue_replicating_decision_policy = PolicyChain(p.will_continue_replicating_decision_policy_def, lua);
  };

  virtual DecisionStrategy &selectOutcomeFromExperiment(Experiment *experiment,
                                    PolicyChainSet &pchain_set) override;
  
  virtual DecisionStrategy &selectOutcomeFromPool(SubmissionPool &spool,
                                    PolicyChainSet &pchain_set) override;

  virtual bool willStartHacking() override;
  
  virtual bool willContinueHacking(PolicyChain &pchain) override;
};

// JSON Parser for MarjansDecisionStrategy::Parameters
inline void to_json(json &j, const MarjansDecisionMaker::Parameters &p) {
  j = json{{"_name", p.name},
           {"initial_selection_policies", p.initial_selection_policies_defs},
           {"submission_decision_policies", p.submission_decision_policies_defs},
           {"between_hacks_selection_policies", p.between_hacks_selection_policies_defs},
           {"between_replications_selection_policies", p.between_replications_selection_policies_defs},
          {"will_start_hacking_decision_policies", p.will_start_hacking_decision_policies_def},
          {"will_continue_replicating_decision_policy", p.will_continue_replicating_decision_policy_def}
  };
}

inline void from_json(const json &j, MarjansDecisionMaker::Parameters &p) {
  j.at("_name").get_to(p.name);
  j.at("initial_selection_policies").get_to(p.initial_selection_policies_defs);
  j.at("submission_decision_policies").get_to(p.submission_decision_policies_defs);
  j.at("between_hacks_selection_policies").get_to(p.between_hacks_selection_policies_defs);
  j.at("between_replications_selection_policies").get_to(p.between_replications_selection_policies_defs);
  j.at("will_start_hacking_decision_policies").get_to(p.will_start_hacking_decision_policies_def);
  j.at("will_continue_replicating_decision_policy").get_to(p.will_continue_replicating_decision_policy_def);
}


} // namespace sam

#endif // SAMPP_DECISIONSTRATEGY_H
