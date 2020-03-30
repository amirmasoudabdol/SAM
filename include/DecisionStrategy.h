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
#include <memory>
#include <nlohmann/json.hpp>
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>
#include <utility>
#include <vector>

#include "sam.h"

#include "Experiment.h"
#include "GroupData.h"
#include "Submission.h"
#include "Utilities.h"

using json = nlohmann::json;

namespace sam {

enum class DecisionMethod {
  HonestDecisionMaker,
  PatientDecisionMaker,
  ImpatientDecisionMaker,
  NoDecision
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    DecisionMethod,
    {{DecisionMethod::HonestDecisionMaker, "HonestDecisionMaker"},
     {DecisionMethod::PatientDecisionMaker, "PatientDecisionMaker"},
     {DecisionMethod::ImpatientDecisionMaker, "ImpatientDecisionMaker"},
     {DecisionMethod::NoDecision, "NoDecision"}})

///
/// DecisionStage enum indicates on what stages of the _research_ the Researcher
/// is making decision in.
///
enum class DecisionStage { Initial, WhileHacking, DoneHacking, Final };

NLOHMANN_JSON_SERIALIZE_ENUM(DecisionStage,
                             {{DecisionStage::Initial, "Initial"},
                              {DecisionStage::WhileHacking, "WhileHacking"},
                              {DecisionStage::DoneHacking, "DoneHacking"},
                              {DecisionStage::Final, "Final"}})

enum class PolicyType { Min, Max, Comp, Random, First, All };

using Policy = std::pair<PolicyType, sol::function>;
using PolicySet = std::vector<Policy>;
using PolicyChain = std::vector<PolicySet>;

using SubmissionPool = std::vector<Submission>;

///
/// \brief      Abstract class for different decision strategies.
///
class DecisionStrategy {

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
  PolicyChain initial_decision_policies;
  PolicySet submission_policies;
  PolicyChain final_decision_policies;

  std::optional<Policy> registerPolicy(const std::string &s);

  PolicySet registerPolicySet(const std::vector<std::string> &policy_set_defs);

  PolicyChain registerPolicyChain(
      const std::vector<std::vector<std::string>> &policy_chain_defs);

  template <typename ForwardIt>
  std::tuple<bool, ForwardIt, ForwardIt>
  checkThePolicy(const ForwardIt &begin, ForwardIt &end, Policy &policy);

  /// The logic of continuation should be implemented here. Researcher will
  /// ask this method to asses the state of its progress.
  virtual bool willBeHacking() = 0;


  //! If `true`, the Researcher will continue traversing through the
  //! hacknig methods, otherwise, he/she will stop the hacking and
  //! prepare the finalSubmission. It will be updated on each call of
  //! verdict(). Basically verdict() decides if the Researcher is
  //! happy with the submission record or not.
  bool is_still_hacking{false};

  bool will_be_submitting{false};

  bool has_a_candidate{false};
  
  bool has_a_final_candidate{false};

  bool hasSubmissionCandidate() const { return has_a_candidate; };


  /// Experiment
  void operator()(Experiment *experiment, PolicyChain &pchain) {
    verdict(experiment, pchain);
  }

  /// Submission Pool
  void operator()(SubmissionPool &spool, PolicyChain &pchain) {
    verdict(spool, pchain);
  }
  
  /// Submission
  bool willBeSubmitting(PolicySet &pset);
  
protected:
  
  /// A helper method to save the current submission. This needs to be called
  /// after verdict.
  void saveCurrentSubmission() {
    submissions_pool.push_back(current_submission_candidate);
  };

  /// Clear the list of submissions and experiments
  void clearHistory() {
    submissions_pool.clear();
  }
  
  void selectOutcome(Experiment &experiment, PolicyChain &pchain);

  void selectBetweenSubmissions(SubmissionPool &spool, PolicyChain &pchain);
  
private:
  
  /// \brief      Implementation of decision-making procedure.
  ///
  /// \param      experiment
  /// \param[in]  stage       The stage in which the researcher is asking
  ///                         for the verdict. The implementation of verdict
  ///                         sould provide different procedure for different
  ///                         stages of the development.
  ///
  /// \return     A boolean indicating whether result is satisfactory or not
  virtual DecisionStrategy &verdict(Experiment *experiment,
                                    PolicyChain &pchain) = 0;
  
  virtual DecisionStrategy &verdict(SubmissionPool &spool,
                                    PolicyChain &pchain) = 0;
  
};

/// \ingroup    DecisionStrategies
///
/// \brief      Implementation of an impatient researcher. In this case, the
///             Researcher will stop as soon as find a significant result and
///             will not continue exploring other hacking methods in his
///             arsenal.
class ImpatientDecisionMaker final : public DecisionStrategy {

public:
  struct Parameters {
    DecisionMethod name = DecisionMethod::ImpatientDecisionMaker;

    std::vector<std::vector<std::string>> initial_decision_policies_defs;
    std::vector<std::string> submission_policies_defs;
  };

  Parameters params;

  explicit ImpatientDecisionMaker(const Parameters &p) {

    spdlog::debug("Registering decision policies...");

    initial_decision_policies = registerPolicyChain(p.initial_decision_policies_defs);

    submission_policies = registerPolicySet(p.submission_policies_defs);
  }

  bool willBeHacking() override { return not has_a_candidate; };

  virtual DecisionStrategy &verdict(Experiment *experiment,
                                    PolicyChain &pchain) override;
  
  virtual DecisionStrategy &verdict(SubmissionPool &spool,
                                    PolicyChain &pchain) override;
};

// JSON Parser for ImpatientDecisionStrategy::Parameters
inline void to_json(json &j, const ImpatientDecisionMaker::Parameters &p) {
  j = json{{"_name", p.name},
           {"initial_decision_policies", p.initial_decision_policies_defs},
           {"submission_policies", p.submission_policies_defs}};
}

inline void from_json(const json &j, ImpatientDecisionMaker::Parameters &p) {
  j.at("_name").get_to(p.name);
  j.at("initial_decision_policies").get_to(p.initial_decision_policies_defs);
  j.at("submission_policies").get_to(p.submission_policies_defs);
}

///
/// \ingroup    DecisionStrategies
/// \brief      { item_description }
///
class PatientDecisionMaker final : public DecisionStrategy {

public:
  struct Parameters {
    DecisionMethod name = DecisionMethod::PatientDecisionMaker;

    std::vector<std::vector<std::string>> initial_decision_policies_defs;
    std::vector<std::string> submission_policies_defs;
    std::vector<std::vector<std::string>> final_decision_policies_defs;
  };

  Parameters params;

  explicit PatientDecisionMaker(const Parameters &p) : params{p} {

    initial_decision_policies = registerPolicyChain(p.initial_decision_policies_defs);

    final_decision_policies = registerPolicyChain(p.final_decision_policies_defs);

    submission_policies = registerPolicySet(p.submission_policies_defs);
  };

  virtual DecisionStrategy &verdict(Experiment *experiment,
                                    PolicyChain &pchain) override;
  
  virtual DecisionStrategy &verdict(SubmissionPool &spool,
                                    PolicyChain &pchain) override;

  // She is always hacking
  virtual bool willBeHacking() override { return true; }
};

// JSON Parser for PatientDecisionStrategy::Parameters
inline void to_json(json &j, const PatientDecisionMaker::Parameters &p) {
  j = json{{"_name", p.name},
           {"initial_decision_policies", p.initial_decision_policies_defs},
           {"submission_policies", p.submission_policies_defs},
           {"final_decision_policies", p.final_decision_policies_defs}};
}

inline void from_json(const json &j, PatientDecisionMaker::Parameters &p) {
  j.at("_name").get_to(p.name);
  j.at("initial_decision_policies").get_to(p.initial_decision_policies_defs);
  j.at("submission_policies").get_to(p.submission_policies_defs);
  j.at("final_decision_policies").get_to(p.final_decision_policies_defs);
}

} // namespace sam

#endif // SAMPP_DECISIONSTRATEGY_H
