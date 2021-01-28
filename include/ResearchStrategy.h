//===-- ResearchStrategy.h - Research Strategy Deceleration ---------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-02-01.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This files contains the deceleration of the Research Strategy class which
/// is responsible for almost all the decision making throughout the research
/// process.
///
//===----------------------------------------------------------------------===//
///
/// @defgroup   ResearchStrategies Research Strategies
/// @brief      List of available Research Strategies
///
/// @defgroup   ResearchStrategyParameters Parameters of the Research Strategies
///

#ifndef SAMPP_RESEARCHSTRATEGY_H
#define SAMPP_RESEARCHSTRATEGY_H

#include <fmt/core.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <sol/sol.hpp>

#include "Distributions.h"
#include "Experiment.h"
#include "Policy.h"

namespace sam {

enum class DecisionMethod { DefaultResearchStrategy };

NLOHMANN_JSON_SERIALIZE_ENUM(DecisionMethod,
                             {{DecisionMethod::DefaultResearchStrategy,
                               "DefaultResearchStrategy"}})

using SubmissionPool = std::vector<Submission>;

///
/// @brief      Abstract class for different research strategies.
///
/// @note       This is being implemented as abstract class due to historical
///             reasons. There is only one research strategy at the moment, but
///             I'm considering the possibility of adding more and maybe
///             redesigning this again.
///
class ResearchStrategy {
 protected:
  std::optional<SubmissionPool> submission_candidates;

  //! List of selected submissions collected by the researcher
  //! using the stashing_policy
  SubmissionPool stashed_submissions;

 public:
  // Lua state
  sol::state lua;

  virtual ~ResearchStrategy() = 0;
  ResearchStrategy();

  /// ResearchStrategy factory method.
  static std::unique_ptr<ResearchStrategy> build(
      json &research_strategy_config);

  /// @todo These guys should move to their own class, I don't have to keep
  /// everything here!
  PolicyChainSet initial_selection_policies;
  PolicyChain submission_decision_policies;
  PolicyChainSet between_stashed_selection_policies;
  PolicyChainSet between_reps_policies;

  PolicyChain will_not_start_hacking_decision_policies;

  PolicyChain will_not_continue_replicating_decision_policy;

  PolicyChain stashing_policy;

  ///
  /// @brief      Indicates whether the researcher will start going to the
  ///             hacking procedure.
  ///
  ///             The default here is to not go for hacking if we already have
  ///             one candidate; but this can be overridden in different
  ///             research strategies
  ///
  /// @param      subs  The subs
  ///
  /// @return     Returns `true` if researcher is going to start the hacking
  ///             procedure
  ///
  virtual bool willStartHacking(std::optional<SubmissionPool> &subs) {
    return false;
  };

  /// @brief      Indicates whether the researcher will continue the hacking
  ///             procedure
  ///
  ///             This will be used by `hackTheResearch` and uses the decision
  ///             policy to decide whether the next hacking strategy is going to
  ///             be executed !
  ///
  /// @param      experiment  A reference to the experiment
  virtual bool willContinueHacking(Experiment *experiment,
                                   PolicyChain &pchain) {
    return false;
  };

  virtual bool willContinueHacking(std::optional<SubmissionPool> &sub,
                                    PolicyChain &pchain) {
    return false;
  };

  /// Indicates whether the Researcher is going to submit the given
  /// set of submissions or not.
  bool willBeSubmitting(const std::optional<SubmissionPool> &sub,
                        PolicyChain &pchain);

  ///
  /// @brief      Decides whether the researcher is going to continue the
  ///             replication process or not.
  ///
  /// @param      subs  A list of submissions
  ///
  /// @return     Returns `true` if the replication procedure should continue
  ///
  virtual bool willContinueReplicating(SubmissionPool &subs) { return false; };

  /// Resets the internal state of the research strategy
  void reset() {
    stashed_submissions.clear();
    submission_candidates.reset();
  }


  /// @brief      Returns a copy of the #stashed_submissions
  ///
  /// @todo       this can be improved
  ///
  /// @return     Returns an optional containing a copy of stashed_submissions
  ///
  std::optional<SubmissionPool> stashedSubmissions() {
    if (stashed_submissions.empty()) {
      return std::nullopt;
    }

    return std::optional<SubmissionPool>{stashed_submissions};
  };

  ///
  /// @brief      Returns an optional result of applying the PolicyChainSet on
  ///             the given Experiment
  ///
  /// @param      experiment  The experiment
  /// @param      pchain_set  The policy chain set
  ///
  /// @return     Returns a list of submissions, if any.
  ///
  virtual std::optional<SubmissionPool> selectOutcomeFromExperiment(
      Experiment *experiment, PolicyChainSet &pchain_set) = 0;

  ///
  /// @brief      Returns an optional result of applying the PolicyChainSet on
  ///             the given list of submissions
  ///
  /// @param      spool       The submission pool
  /// @param      pchain_set  The policy chain set
  ///
  /// @return     Returns a list of submissions, if any.
  ///
  virtual std::optional<SubmissionPool> selectOutcomeFromPool(
      SubmissionPool &spool, PolicyChainSet &pchain_set) = 0;

  /// Creates and save all possible submissions from an experiment, if they pass
  /// the given policy predicate
  ///
  /// @param      experiment  a reference to the experiment
  /// @param      pchain      a policy chain, usually #stashing_policy
  void saveOutcomes(Experiment &experiment, PolicyChain &pchain);

  ///
  /// @brief      Returns the result of applying the PolicyChainSet on the
  ///             Experiment
  ///
  /// @param      experiment  The experiment
  /// @param      pchain_set  The policy chain set.
  ///
  /// @return     Returns a list of submissions, if any.
  ///
  std::optional<SubmissionPool> selectOutcome(Experiment &experiment,
                                              PolicyChainSet &pchain_set);

  ///
  /// @brief      Returns the result of applying the PolicyChainSet on the
  ///             submission pool
  ///
  /// @param      spool       The spool
  /// @param      pchain_set  The policy chain set
  ///
  /// @return     Returns a list of submissions, if any.
  ///
  std::optional<SubmissionPool> selectBetweenSubmissions(
      SubmissionPool &spool, PolicyChainSet &pchain_set);
};

/// @brief      The deceleration of the default research strategy
///
///             The default research strategy is the only available strategy at
///             the moment. It is designed to use all the policies explicitly
///             and do not intervene with the given logic. Therefore, it has a
///             very linear flow.
///
/// @note       Researcher is in charge of research strategy and it executes its
///             command personally.
///
/// @see        Researcher::research()
///
/// @ingroup    ResearchStrategies
class DefaultResearchStrategy final : public ResearchStrategy {
 public:

  ///
  /// @brief      The parameters of the strategy
  ///
  /// @ingroup    ResearchStrategyParameters
  ///
  struct Parameters {
    DecisionMethod name = DecisionMethod::DefaultResearchStrategy;

    //! Initial Selection Policy
    std::vector<std::vector<std::string>> initial_selection_policies_defs;

    //! Will Start Hacking Decision Policy
    std::vector<std::string> will_not_start_hacking_decision_policies_def;

    //! Between Stashed Selection Policy
    std::vector<std::vector<std::string>>
        between_stashed_selection_policies_defs;

    //! Will Continue Replicating Decision Policy
    std::vector<std::string> will_not_continue_replicating_decision_policy_def;

    //! Between Replication Selection Policy
    std::vector<std::vector<std::string>>
        between_replications_selection_policies_defs;

    //! Stashing Selection Policy
    std::vector<std::string> stashing_policy_def;

    //! Submissions Decision Policy
    std::vector<std::string> submission_decision_policies_defs;
  };

  Parameters params;

  explicit DefaultResearchStrategy(const Parameters &p) : params{p} {

    spdlog::trace("Preparing Initial Selection Policies: ");
    initial_selection_policies =
        PolicyChainSet(p.initial_selection_policies_defs, lua);

    spdlog::trace("Preparing Will Start Hacking Decision Policies: ");
    will_not_start_hacking_decision_policies =
        PolicyChain(p.will_not_start_hacking_decision_policies_def,
                    PolicyChainType::Decision, lua);

    spdlog::trace("Preparing Between Stashed Selection Policies: ");
    between_stashed_selection_policies =
        PolicyChainSet(p.between_stashed_selection_policies_defs, lua);


    spdlog::trace("Preparing Between Replication Selection Policies: ");
    between_reps_policies =
        PolicyChainSet(p.between_replications_selection_policies_defs, lua);


    spdlog::trace("Preparing Will Continue Replicating Decision Policies: ");
    will_not_continue_replicating_decision_policy =
        PolicyChain(p.will_not_continue_replicating_decision_policy_def,
                    PolicyChainType::Decision, lua);

    spdlog::trace("Preparing Stashing Policies: ");
    stashing_policy =
        PolicyChain(p.stashing_policy_def, PolicyChainType::Selection, lua);

    spdlog::trace("Preparing Submission Decision Policies: ");
    submission_decision_policies = PolicyChain(
        p.submission_decision_policies_defs, PolicyChainType::Decision, lua);
  };

  /// Returns the result of applying the policy chain set on the Experiment
  std::optional<SubmissionPool> selectOutcomeFromExperiment(
      Experiment *experiment, PolicyChainSet &pchain_set) override;

  /// Returns the results of applying the policy chain set on the list of 
  /// submissions
  std::optional<SubmissionPool> selectOutcomeFromPool(
      SubmissionPool &spool, PolicyChainSet &pchain_set) override;

  /// Determines whether or not the researcher is going to starting the
  /// hacking / procedure
  bool willStartHacking(std::optional<SubmissionPool> &subs) override;

  /// Determines whether or not the researcher is going to continue hacking
  bool willContinueHacking(Experiment *experiment,
                           PolicyChain &pchain) override;

  /// Determines whether or not the researcher is going to continue hacking
  bool willContinueHacking(std::optional<SubmissionPool> &subs,
                           PolicyChain &pchain) override;

  /// Determines whether or not the replication procedure is going to place
  bool willContinueReplicating(SubmissionPool &subs) override;
};

// JSON Parser for DefaultResearchStrategy::Parameters
inline void to_json(json &j, const DefaultResearchStrategy::Parameters &p) {
  j = json{
      {"name", p.name},
      {"initial_selection_policies", p.initial_selection_policies_defs},
      {"submission_decision_policies", p.submission_decision_policies_defs},
      {"between_stashed_selection_policies",
       p.between_stashed_selection_policies_defs},
      {"between_replications_selection_policies",
       p.between_replications_selection_policies_defs},
      {"will_not_start_hacking_decision_policies",
       p.will_not_start_hacking_decision_policies_def},
      {"will_not_continue_replicating_decision_policy",
       p.will_not_continue_replicating_decision_policy_def},
      {"stashing_policy", p.stashing_policy_def}};
}

inline void from_json(const json &j, DefaultResearchStrategy::Parameters &p) {
  j.at("name").get_to(p.name);
  j.at("initial_selection_policies").get_to(p.initial_selection_policies_defs);
  j.at("submission_decision_policies")
      .get_to(p.submission_decision_policies_defs);
  j.at("between_stashed_selection_policies")
      .get_to(p.between_stashed_selection_policies_defs);
  j.at("between_replications_selection_policies")
      .get_to(p.between_replications_selection_policies_defs);
  j.at("will_not_start_hacking_decision_policies")
      .get_to(p.will_not_start_hacking_decision_policies_def);
  j.at("will_not_continue_replicating_decision_policy")
      .get_to(p.will_not_continue_replicating_decision_policy_def);
  j.at("stashing_policy").get_to(p.stashing_policy_def);
}

}  // namespace sam

#endif  // SAMPP_RESEARCHSTRATEGY_H
