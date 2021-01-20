//===-- DecisionStrategy.h - Decision Strategy Deceleration ---------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-02-01.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This files contains the deceleration of the Decision Strategy class which
/// is responsible for almost all the decision making throughout the research
/// process.
///
//===----------------------------------------------------------------------===//
///
/// @defgroup   DecisionStrategies Decision Strategies
/// @brief      List of available Decision Strategies
///

#ifndef SAMPP_DECISIONSTRATEGY_H
#define SAMPP_DECISIONSTRATEGY_H

#include <spdlog/spdlog.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <sol/sol.hpp>

#include "Experiment.h"
#include "Distributions.h"
#include "Policy.h"

namespace sam {

enum class DecisionMethod {
  DefaultDecisionMaker
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    DecisionMethod,
    {{DecisionMethod::DefaultDecisionMaker, "DefaultDecisionMaker"}})

using SubmissionPool = std::vector<Submission>;


///
/// @brief      Abstract class for different decision strategies.
///
class DecisionStrategy {

public:
  // Lua state
  sol::state lua;
  
  virtual ~DecisionStrategy() = 0;
  DecisionStrategy();
  
  /// DecisionStrategy factory method.
  ///
  /// @param decision_strategy_config    A JSON object containing information
  /// about each decision strategy.
  static std::unique_ptr<DecisionStrategy>
  build(json &decision_strategy_config);
  
  std::optional<SubmissionPool> submission_candidates;
  
  //! List of selected submissions collected by the researcher
  //! using the stashing_policy
  SubmissionPool stashed_submissions;
  
  //! List of submissions selecting during the hacking procedure
  SubmissionPool hacked_submissions;

  /// @todo These guys should move to their own class, I don't have to keep everything here!
  PolicyChainSet initial_selection_policies;
  PolicyChain submission_decision_policies;
  PolicyChainSet between_stashed_selection_policies;
  PolicyChainSet between_reps_policies;
  
  PolicyChain will_start_hacking_decision_policies;

  PolicyChain will_continue_replicating_decision_policy;
  
  PolicyChain stashing_policy;
  
  
  /// @brief  Indicates whether the researcher will start going to the hacking procedure.
  /// The default here is to not go for hacking if we already have one candidate; but this
  /// can be overridden in different decision strategies
  virtual bool willStartHacking() { return false; };
  
  /// @brief  This will be used by `letTheHackBegin` and uses the decision policy to decide
  /// whether the next hacking strategy is going to be executed !
  ///
  /// @param  experiment A reference to the experiment
  virtual bool willContinueHacking(Experiment *experiment,
                                   PolicyChain &pchain) {return false; };
  
  
  /// Submission
  bool willBeSubmitting(const std::optional<SubmissionPool>& sub,
                        PolicyChain &pchain);
  
  virtual bool willContinueReplicating(PolicyChain &pchain) {return false; };
  
  /// @todo This needs to be private but currently, I don't have a good place to put it.
  /// The verdict system is broken, and if reset it after the selectionBetweenSubmission, it's weird
  /// and I cannot just call it in any other methods because then it's hidden
  ///
  /// Reset the internal state of the decision strategy
  void reset() {
    stashed_submissions.clear();
    submission_candidates.reset();
//    clear();
  }
  
  /// @brief      Implementation of decision-making procedure.
  virtual DecisionStrategy &selectOutcomeFromExperiment(Experiment *experiment,
                                    PolicyChainSet &pchain_set) = 0;
  
  virtual DecisionStrategy &selectOutcomeFromPool(SubmissionPool &spool,
                                    PolicyChainSet &pchain_set) = 0;

  /// Create and save all possible submissions from an experiment, if they pass
  /// the given policy predicate
  ///
  /// @param experiment a reference to the experiment
  /// @param pchain a policy chain, usually stored in `stashing_policy` in the config file
  void saveOutcomes(Experiment &experiment, PolicyChain &pchain);
  
protected:
  
  void selectOutcome(Experiment &experiment, PolicyChainSet &pchain_set);

  void selectBetweenSubmissions(SubmissionPool &spool, PolicyChainSet &pchain_set);

  
};

///
/// @ingroup    DecisionStrategies
class DefaultDecisionMaker final : public DecisionStrategy {

public:
  struct Parameters {
    DecisionMethod name = DecisionMethod::DefaultDecisionMaker;

    std::vector<std::vector<std::string>> initial_selection_policies_defs;
    std::vector<std::string> submission_decision_policies_defs;
    std::vector<std::vector<std::string>> between_stashed_selection_policies_defs;
    std::vector<std::vector<std::string>> between_replications_selection_policies_defs;
    
    std::vector<std::string> will_start_hacking_decision_policies_def;
    
    std::vector<std::string> will_continue_replicating_decision_policy_def;
    
    std::vector<std::string> stashing_policy_def;
  };

  Parameters params;

  explicit DefaultDecisionMaker(const Parameters &p) : params{p} {

    spdlog::trace("Preparing Initial Selection Policies: ");
    initial_selection_policies = PolicyChainSet(p.initial_selection_policies_defs, lua);

    spdlog::trace("Preparing Between Stashed Selection Policies: ");
    between_stashed_selection_policies = PolicyChainSet(p.between_stashed_selection_policies_defs, lua);

    spdlog::trace("Preparing Submission Decision Policies: ");
    submission_decision_policies = PolicyChain(p.submission_decision_policies_defs, lua);
    
    spdlog::trace("Preparing Between Replication Selection Policies: ");
    between_reps_policies = PolicyChainSet(p.between_replications_selection_policies_defs, lua);
    
    spdlog::trace("Preparing Will Start Hacking Decision Policies: ");
    will_start_hacking_decision_policies = PolicyChain(p.will_start_hacking_decision_policies_def, lua);
    
    spdlog::trace("Preparing Will Conitnue Replicating Decision Policies: ");
    will_continue_replicating_decision_policy = PolicyChain(p.will_continue_replicating_decision_policy_def, lua);
    
    spdlog::trace("Preparing Stashing Policies: ");
    stashing_policy = PolicyChain(p.stashing_policy_def, lua);
  };

  DecisionStrategy &selectOutcomeFromExperiment(Experiment *experiment,
                                    PolicyChainSet &pchain_set) override;
  
  DecisionStrategy &selectOutcomeFromPool(SubmissionPool &spool,
                                    PolicyChainSet &pchain_set) override;

  bool willStartHacking() override;
  
  bool willContinueHacking(Experiment *experiment,
                                   PolicyChain &pchain) override;
  
  bool willContinueReplicating(PolicyChain &pchain) override;
};

// JSON Parser for DefaultDecisionStrategy::Parameters
inline void to_json(json &j, const DefaultDecisionMaker::Parameters &p) {
  j = json{{"name", p.name},
           {"initial_selection_policies", p.initial_selection_policies_defs},
           {"submission_decision_policies", p.submission_decision_policies_defs},
           {"between_stashed_selection_policies", p.between_stashed_selection_policies_defs},
           {"between_replications_selection_policies", p.between_replications_selection_policies_defs},
          {"will_start_hacking_decision_policies", p.will_start_hacking_decision_policies_def},
          {"will_continue_replicating_decision_policy", p.will_continue_replicating_decision_policy_def},
          {"stashing_policy", p.stashing_policy_def}
  };
}

inline void from_json(const json &j, DefaultDecisionMaker::Parameters &p) {
  j.at("name").get_to(p.name);
  j.at("initial_selection_policies").get_to(p.initial_selection_policies_defs);
  j.at("submission_decision_policies").get_to(p.submission_decision_policies_defs);
  j.at("between_stashed_selection_policies").get_to(p.between_stashed_selection_policies_defs);
  j.at("between_replications_selection_policies").get_to(p.between_replications_selection_policies_defs);
  j.at("will_start_hacking_decision_policies").get_to(p.will_start_hacking_decision_policies_def);
  j.at("will_continue_replicating_decision_policy").get_to(p.will_continue_replicating_decision_policy_def);
  j.at("stashing_policy").get_to(p.stashing_policy_def);
}


} // namespace sam

#endif // SAMPP_DECISIONSTRATEGY_H
