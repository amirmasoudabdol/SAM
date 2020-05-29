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
  HonestDecisionMaker,
  PatientDecisionMaker,
  ImpatientDecisionMaker,
  MarjansDecisionMaker,
  NoDecision
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    DecisionMethod,
    {{DecisionMethod::HonestDecisionMaker, "HonestDecisionMaker"},
     {DecisionMethod::PatientDecisionMaker, "PatientDecisionMaker"},
     {DecisionMethod::ImpatientDecisionMaker, "ImpatientDecisionMaker"},
     {DecisionMethod::MarjansDecisionMaker, "MarjansDecisionMaker"},
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
  
  PolicyChain will_be_hacking_decision_policies;




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
  void operator()(Experiment *experiment, PolicyChainSet &pchain_set) {
    verdict(experiment, pchain_set);
  }

  /// Submission Pool
  void operator()(SubmissionPool &spool, PolicyChainSet &pchain_set) {
    verdict(spool, pchain_set);
  }
  
  /// Submission
  bool willBeSubmitting(PolicyChain &pchain);
  
  /// Clear the contents of the decision strategy, this include the
  /// submission pools or other collected information by the decision
  /// strategy.
  void clear() {
    submissions_pool.clear();
  }
  
  // TODO: This needs to be private but currently, I don't have a good place to put it.
  // The verdict system is broken, and if reset it after the selectionBetweenSubmission, it's werid
  // and I cannot just call it in any other methods because then it's hidden
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
  
  virtual bool willBeHacking(Experiment &experiment) { return not has_any_candidates; };
  
  
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
                                    PolicyChainSet &pchain_set) = 0;
  
  virtual DecisionStrategy &verdict(SubmissionPool &spool,
                                    PolicyChainSet &pchain_set) = 0;
  
protected:
  
  /// A helper method to save the current submission. This needs to be called
  /// after verdict.
  void saveCurrentSubmission() {
    submissions_pool.push_back(current_submission_candidate);
  };
  
  void saveEverySubmission(Experiment &experiment) {
    for (int i{experiment.setup.nd()}, d{0}; i < experiment.setup.ng();
         ++i, ++d %= experiment.setup.nd()) {
      submissions_pool.emplace_back(experiment, i);
    }
  };
  
  void selectOutcome(Experiment &experiment, PolicyChainSet &pchain_set);

  void selectBetweenSubmissions(SubmissionPool &spool, PolicyChainSet &pchain_set);

  
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

    std::vector<std::vector<std::string>> initial_selection_policies_defs;
    std::vector<std::string> submission_decision_policies_defs;
  };

  Parameters params;

  explicit ImpatientDecisionMaker(const Parameters &p) {

    spdlog::debug("Registering decision policies...");

    initial_selection_policies = PolicyChainSet(p.initial_selection_policies_defs, lua);

    submission_decision_policies = PolicyChain(p.submission_decision_policies_defs, lua);
  }

  bool willBeHacking(Experiment &experiment) override { return not has_any_candidates; };

  virtual DecisionStrategy &verdict(Experiment *experiment,
                                    PolicyChainSet &pchain_set) override;
  
  virtual DecisionStrategy &verdict(SubmissionPool &spool,
                                    PolicyChainSet &pchain_set) override;
};

// JSON Parser for ImpatientDecisionStrategy::Parameters
inline void to_json(json &j, const ImpatientDecisionMaker::Parameters &p) {
  j = json{{"_name", p.name},
           {"initial_selection_policies", p.initial_selection_policies_defs},
           {"submission_decision_policies", p.submission_decision_policies_defs}};
}

inline void from_json(const json &j, ImpatientDecisionMaker::Parameters &p) {
  j.at("_name").get_to(p.name);
  j.at("initial_selection_policies").get_to(p.initial_selection_policies_defs);
  j.at("submission_decision_policies").get_to(p.submission_decision_policies_defs);
}

///
/// \ingroup    DecisionStrategies
/// \brief      { item_description }
///
class PatientDecisionMaker final : public DecisionStrategy {

public:
  struct Parameters {
    DecisionMethod name = DecisionMethod::PatientDecisionMaker;

    std::vector<std::vector<std::string>> initial_selection_policies_defs;
    std::vector<std::string> submission_decision_policies_defs;
    std::vector<std::vector<std::string>> between_hacks_selection_policies_defs;
    std::vector<std::vector<std::string>> between_replications_selection_policies_defs;
  };

  Parameters params;

  explicit PatientDecisionMaker(const Parameters &p) : params{p} {

    initial_selection_policies = PolicyChainSet(p.initial_selection_policies_defs, lua);

    between_hacks_selection_policies = PolicyChainSet(p.between_hacks_selection_policies_defs, lua);

    submission_decision_policies = PolicyChain(p.submission_decision_policies_defs, lua);
    
    between_reps_policies = PolicyChainSet(p.between_replications_selection_policies_defs, lua);
  };

  virtual DecisionStrategy &verdict(Experiment *experiment,
                                    PolicyChainSet &pchain_set) override;
  
  virtual DecisionStrategy &verdict(SubmissionPool &spool,
                                    PolicyChainSet &pchain_set) override;

  // She is always hacking
  virtual bool willBeHacking(Experiment &experiment) override { return true; }
};

// JSON Parser for PatientDecisionStrategy::Parameters
inline void to_json(json &j, const PatientDecisionMaker::Parameters &p) {
  j = json{{"_name", p.name},
           {"initial_selection_policies", p.initial_selection_policies_defs},
           {"submission_decision_policies", p.submission_decision_policies_defs},
           {"between_hacks_selection_policies", p.between_hacks_selection_policies_defs},
           {"between_replications_selection_policies", p.between_replications_selection_policies_defs}
  };
}

inline void from_json(const json &j, PatientDecisionMaker::Parameters &p) {
  j.at("_name").get_to(p.name);
  j.at("initial_selection_policies").get_to(p.initial_selection_policies_defs);
  j.at("submission_decision_policies").get_to(p.submission_decision_policies_defs);
  j.at("between_hacks_selection_policies").get_to(p.between_hacks_selection_policies_defs);
  j.at("between_replications_selection_policies").get_to(p.between_replications_selection_policies_defs);
}



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
    
    std::vector<std::string> will_be_hacking_decision_policies_def;
  };

  Parameters params;

  explicit MarjansDecisionMaker(const Parameters &p) : params{p} {

    initial_selection_policies = PolicyChainSet(p.initial_selection_policies_defs, lua);

    between_hacks_selection_policies = PolicyChainSet(p.between_hacks_selection_policies_defs, lua);

    submission_decision_policies = PolicyChain(p.submission_decision_policies_defs, lua);
    
    between_reps_policies = PolicyChainSet(p.between_replications_selection_policies_defs, lua);
    
    will_be_hacking_decision_policies = PolicyChain(p.will_be_hacking_decision_policies_def, lua);
  };

  virtual DecisionStrategy &verdict(Experiment *experiment,
                                    PolicyChainSet &pchain_set) override;
  
  virtual DecisionStrategy &verdict(SubmissionPool &spool,
                                    PolicyChainSet &pchain_set) override;

  virtual bool willBeHacking(Experiment &experiment) override;
  
  bool willContinueHacking(Experiment &expriment, PolicyChain &pchain);
};

// JSON Parser for PatientDecisionStrategy::Parameters
inline void to_json(json &j, const MarjansDecisionMaker::Parameters &p) {
  j = json{{"_name", p.name},
           {"initial_selection_policies", p.initial_selection_policies_defs},
           {"submission_decision_policies", p.submission_decision_policies_defs},
           {"between_hacks_selection_policies", p.between_hacks_selection_policies_defs},
           {"between_replications_selection_policies", p.between_replications_selection_policies_defs},
          {"will_be_hacking_decision_policies", p.will_be_hacking_decision_policies_def}
  };
}

inline void from_json(const json &j, MarjansDecisionMaker::Parameters &p) {
  j.at("_name").get_to(p.name);
  j.at("initial_selection_policies").get_to(p.initial_selection_policies_defs);
  j.at("submission_decision_policies").get_to(p.submission_decision_policies_defs);
  j.at("between_hacks_selection_policies").get_to(p.between_hacks_selection_policies_defs);
  j.at("between_replications_selection_policies").get_to(p.between_replications_selection_policies_defs);
  j.at("will_be_hacking_decision_policies").get_to(p.will_be_hacking_decision_policies_def);
}


} // namespace sam

#endif // SAMPP_DECISIONSTRATEGY_H
