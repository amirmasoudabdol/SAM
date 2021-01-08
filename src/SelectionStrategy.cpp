//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <stdexcept>

#include "SelectionStrategy.h"

using namespace sam;

SelectionStrategy::~SelectionStrategy() {
  // Pure deconstructor
}

SelectionStrategy::SelectionStrategy() {
  lua.open_libraries();
  
  lua.new_usertype<Group>("GroupData", "id", &Group::id_, "nobs",
                          &Group::nobs_, "pvalue", &Group::pvalue_,
                          "effect", &Group::effect_, "sig",
                          &Group::sig_);
  
  lua.new_usertype<Submission>(
                               "Submission", "id",
                               sol::property([](Submission &s) { return s.group_.id_; }), "nobs",
                               sol::property([](Submission &s) { return s.group_.nobs_; }), "mean",
                               sol::property([](Submission &s) { return s.group_.mean_; }), "pvalue",
                               sol::property([](Submission &s) { return s.group_.pvalue_; }), "effect",
                               sol::property([](Submission &s) { return s.group_.effect_; }), "sig",
                               sol::property([](Submission &s) { return s.group_.sig_; }));
}

std::unique_ptr<SelectionStrategy>
SelectionStrategy::build(json &selection_strategy_config) {

  if (selection_strategy_config["name"] == "PolicyBasedSelection") {

    auto params =
        selection_strategy_config.get<PolicyBasedSelection::Parameters>();
    return std::make_unique<PolicyBasedSelection>(params);

  } else if (selection_strategy_config["name"] == "SignificantSelection") {
    
    auto params =
    selection_strategy_config.get<SignificantSelection::Parameters>();
    return std::make_unique<SignificantSelection>(params);
    
  } else if (selection_strategy_config["name"] == "RandomSelection") {

    auto params = selection_strategy_config.get<RandomSelection::Parameters>();
    return std::make_unique<RandomSelection>(params);

  } else if (selection_strategy_config["name"] == "FreeSelection") {

    return std::make_unique<FreeSelection>();
    
  } else {
    throw std::invalid_argument("Unknown Selection Strategy.");
  }
}


/// @brief  Reviewing the list of submissions based on the user-defined policy
///
/// Check wheter the selection_policy passes, if so, and a
/// random draw from U(0, 1) is true, then the submission will
/// be accepted; otherwise, it will be rejected
///
bool PolicyBasedSelection::review(const std::vector<Submission> &subs) {
  
  /// Checks selection policy returns anything
  /// @todo I don't really like the const_cast here, maybe I need to find a way to
  /// remove it!
  auto check = selection_policy(const_cast<std::vector<Submission>&>(subs));
  if (check and
        Random::get<bool>(params.acceptance_rate) and
          Random::get<bool>(1 - params.pub_bias))
    return true;

  return false;
}



///
/// Check if `p-value` of the Submission is less than the specified
/// \f$\alpha\f$. If true, it will accept the submission, if not, a random
/// number decide wheather the submission is going to be accepted. The
/// drawn random number, \f$r\f$ will be compared to `pub_bias` of the journal.
///
/// @param      s     A reference to the Submission
/// @return     a boolean indicating whether the Submission is accepted.
///
bool SignificantSelection::review(const std::vector<Submission> &subs) {

  // Only accepting +/- results if journal cares about it, side != 0
  if (params.side != 0 and
      std::any_of(subs.begin(), subs.end(),
                  [&](auto &s){return s.group_.eff_side_ != params.side; })
      ) {
    return false;
  }

  /// Checking whether any of the outcomes are significant
  if (std::any_of(subs.begin(), subs.end(),
                  [&](auto &s){return s.group_.pvalue_ < params.alpha; })) {
    return true;
  } else if (Random::get<bool>(1 - params.pub_bias)) {
    return true;
  } else {
    return false;
  }
}

///
/// Draw a random number between \f$ r \in [0, 1] \f$, reject the submission if
/// \f$ r < 0.5 \f$.
///
/// @param      s     corresponding submission
/// @return     a boolean indicating whether the Submission is accpeted.
///
bool RandomSelection::review(const std::vector<Submission> &subs) {
  return Random::get<bool>(0.5);
}
