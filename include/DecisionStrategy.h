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

enum class PolicyType { Min, Max, Comp, Random, First };


using Policy = std::pair<PolicyType, sol::function>;
using PolicySet = std::vector<Policy>;
using PolicyChain = std::vector<PolicySet>;

///
/// \brief      Abstract class for different decision strategies.
///
class DecisionStrategy {

protected:


  json config_;

  //! Indicates the pre-registered outcome in the case where the
  //! Researcher prefers the PreRegisteredOutcome
  int pre_registered_group;

  //! List of selected Submission by the researcher.
  std::vector<Submission> submissions_pool;

  //! List of selected Experiment by the researcher.
  std::vector<Experiment> experiments_pool;

  // Lua state
  sol::state lua;

public:
  
  virtual ~DecisionStrategy() = 0;
  
  DecisionStrategy();

  
  // This should not be here, but I don't feel fixing include collision now!
  std::optional<std::pair<PolicyType, sol::function>>
  registerPolicy(const std::string &s);

  PolicySet
  registerPolicySet(const std::vector<std::string> &policy_set_defs);
  
  PolicyChain
  registerPolicyChain(const std::vector<std::vector<std::string>> &policy_chain_defs);
  
  /// TODO: These guys should move to their own class, I don't have to keep
  /// everything here!
  PolicyChain initial_decision_policies;
  PolicySet submission_policies;
  PolicySet final_decision_policies;
  

  /// DecisionStrategy factory method.
  ///
  /// \param decision_strategy_config    A JSON object containing information
  /// about each decision strategy.
  static std::unique_ptr<DecisionStrategy>
  build(json &decision_strategy_config);


  

  template <typename ForwardIt>
  std::tuple<bool, ForwardIt, ForwardIt>
  checkThePolicy(const ForwardIt &begin, ForwardIt &end, Policy &policy);

  /// The logic of continoution should be implemented here. Researcher will
  /// ask this method to asses the state of its progress.
  virtual bool isStillHacking() { return is_still_hacking; }

  /// It indicates whether the researcher is going to commit to submitting the
  /// _Submission_. This acts as an another level of decision making where the
  /// researcher consider another criteria if it couldn't achieve what he was
  /// "looking for".
  ///
  /// For instance, if the researcher is determined to find "MinSigPvalue"
  /// during his research, and — after all the hacking — he couldn't find
  /// anything significant, then he decide whether he wants to submit the
  /// "unpreferable" result or not.
  virtual bool willBeSubmitting(PolicySet &pset);

//  DecisionStage current_stage;

  Submission current_submission_candidate;
  
  
  //! If `true`, the Researcher will continue traversing through the
  //! hacknig methods, otherwise, he/she will stop the hacking and
  //! prepare the finalSubmission. It will be updated on each call of
  //! verdict(). Basically verdict() decides if the Researcher is
  //! happy with the submission record or not.
  bool is_still_hacking {false};

  bool will_be_submitting {false};
  
  bool has_a_candidate {false};
  
  
  bool hasSubmissionCandidate() const { return has_a_candidate; };

  //! This will set to the final submission record that the Researcher
  //! is satisfied with.
  Submission final_submission;

  /// Clear the list of submissions and experiments
  void clearHistory() {
    submissions_pool.clear();
    experiments_pool.clear();
  }
  
  
  void operator()(Experiment *experiment, PolicyChain &policies) {
    // return selectSubmissions(experiment, policies)
    
    selectOutcome(*experiment, policies);
    
    /// Decision will be blind to the stage with this implementation,
    /// and only execute the routine. It can pass the selected Submission
    /// to the Researccher or just keep it
  }
  
  void operator()(Submission &subs, PolicySet &policies) { }
  
  void operator()(PolicySet &policies) {
    return selectBetweenSubmissions(policies);
    
    /// This should do something!
    
    /// This is quite similar to the previous run, but only operators on subs
  }
  
  
  void operator()(std::vector<Submission> &subs, PolicyChain &policies) {
    // return selectBetweenSubmissions(subs, policies)
    
    /// This is quite similar to the previous run, but only operators on subs
  }
  

  /// \brief      Implementation of decision-making procedure.
  ///
  /// \param      experiment
  /// \param[in]  stage       The stage in which the researcher is asking
  ///                         for the verdict. The implementation of verdict
  ///                         sould provide different procedure for different
  ///                         stages of the development.
  ///
  /// \return     A boolean indicating whether result is satisfactory or not
  virtual DecisionStrategy &verdict(Experiment &experiment,
                                    DecisionStage stage) = 0;

  /// A helper method to save the current submission. This needs to be called
  /// after verdict.
  void saveCurrentSubmission() {
    submissions_pool.push_back(current_submission_candidate);
  };

  ///
  /// \brief      Based on the DecisionPreference, it'll select the outcome
  /// between
  ///             all groups, `ng`. For instance, the MinPvalue deicison
  ///             prefenrece will
  ///
  /// \param      experiment  { parameter_description }
  ///
  /// \return     A copy of the selected outcome
  ///
  Submission selectOutcome(Experiment &experiment);
  
  void selectOutcome(Experiment &experiment, PolicyChain &pchain);

  ///
  /// \brief      Select the final submission by checking all logged
  /// Submissions.
  ///             Each submission is from a hacked experiment by the researcher.
  ///             This is often being used by the `PatientDecisionMaker` at the
  ///             end of the hacking procedure.
  ///
  /// \return     A copy of the selected outcome
  ///
  Submission selectBetweenSubmissions();
  
  
  void selectBetweenSubmissions(PolicySet &pset);

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

    std::vector<std::vector<std::string>> decision_policies;
    std::vector<std::string> submission_policies;
  };

  Parameters params;

  explicit ImpatientDecisionMaker(const Parameters &p) {

    spdlog::debug("Registering decision policies...");

    initial_decision_policies = registerPolicyChain(p.decision_policies);
        
    submission_policies = registerPolicySet(p.submission_policies);
    
  }

  bool isStillHacking() override { return is_still_hacking; }

  virtual ImpatientDecisionMaker &verdict(Experiment &experiment,
                                          DecisionStage stage) override;

};

// JSON Parser for ImpatientDecisionStrategy::Parameters
inline void to_json(json &j, const ImpatientDecisionMaker::Parameters &p) {
  j = json{
      {"_name", p.name},
      {"decision_policies", p.decision_policies},
      {"submission_policies", p.submission_policies}
  };
}

inline void from_json(const json &j, ImpatientDecisionMaker::Parameters &p) {
  j.at("_name").get_to(p.name);
  j.at("decision_policies").get_to(p.decision_policies);
  j.at("submission_policies").get_to(p.submission_policies);
}

///
/// \ingroup    DecisionStrategies
/// \brief      { item_description }
///
class PatientDecisionMaker final : public DecisionStrategy {

public:
  struct Parameters {
    DecisionMethod name = DecisionMethod::PatientDecisionMaker;

    std::vector<std::vector<std::string>> decision_policies;
    std::vector<std::string> submission_policies;
    std::vector<std::string> final_decision_policies;
  };

  Parameters params;

  explicit PatientDecisionMaker(const Parameters &p) : params{p} {

    initial_decision_policies = registerPolicyChain(p.decision_policies);
    
    final_decision_policies = registerPolicySet(p.final_decision_policies);
        
    submission_policies = registerPolicySet(p.submission_policies);

  };

  virtual PatientDecisionMaker &verdict(Experiment &experiment,
                                        DecisionStage stage) override;
  
};

// JSON Parser for PatientDecisionStrategy::Parameters
inline void to_json(json &j, const PatientDecisionMaker::Parameters &p) {
  j = json{{"_name", p.name},
           {"decision_policies", p.decision_policies},
           {"submission_policies", p.submission_policies},
           {"final_decision_policies", p.final_decision_policies}};
}

inline void from_json(const json &j, PatientDecisionMaker::Parameters &p) {
  j.at("_name").get_to(p.name);
  j.at("decision_policies").get_to(p.decision_policies);
  j.at("submission_policies").get_to(p.submission_policies);
  j.at("final_decision_policies").get_to(p.final_decision_policies);
}


} // namespace sam

#endif // SAMPP_DECISIONSTRATEGY_H
