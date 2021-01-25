//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <stdexcept>

#include "ReviewStrategy.h"

using namespace sam;

ReviewStrategy::~ReviewStrategy() {
  // Pure destructors
}

ReviewStrategy::ReviewStrategy() {
  lua.open_libraries();

  lua.new_usertype<DependentVariable>("DependentVariable",
   "id", &DependentVariable::id_,
   "nobs", &DependentVariable::nobs_,
   "mean", &DependentVariable::mean_,
   "pvalue", &DependentVariable::pvalue_,
   "effect", &DependentVariable::effect_,
   "sig", &DependentVariable::sig_,
   "hacked", &DependentVariable::is_hacked_,
   "candidate", &DependentVariable::is_candidate_
   );

  lua.new_usertype<Submission>("Submission",
      "id", sol::property([](Submission &s) { return s.dv_.id_; }),
      "nobs", sol::property([](Submission &s) { return s.dv_.nobs_; }),
      "mean", sol::property([](Submission &s) { return s.dv_.mean_; }),
      "pvalue", sol::property([](Submission &s) { return s.dv_.pvalue_; }),
      "effect", sol::property([](Submission &s) { return s.dv_.effect_; }),
      "sig", sol::property([](Submission &s) { return s.dv_.sig_; }),
      "hacked", sol::property([](Submission &s) { return s.dv_.is_hacked_; }),
      "candidate", sol::property([](Submission &s) { return s.dv_.is_candidate_; }));
}

std::unique_ptr<ReviewStrategy>
ReviewStrategy::build(json &review_strategy_config) {

  if (review_strategy_config["name"] == "PolicyBasedSelection") {

    auto params =
        review_strategy_config.get<PolicyBasedSelection::Parameters>();
    return std::make_unique<PolicyBasedSelection>(params);

  } else if (review_strategy_config["name"] == "SignificantSelection") {

    auto params =
        review_strategy_config.get<SignificantSelection::Parameters>();
    return std::make_unique<SignificantSelection>(params);

  } else if (review_strategy_config["name"] == "RandomSelection") {

    auto params = review_strategy_config.get<RandomSelection::Parameters>();
    return std::make_unique<RandomSelection>(params);

  } else if (review_strategy_config["name"] == "FreeSelection") {

    return std::make_unique<FreeSelection>();

  } else {
    throw std::invalid_argument("Unknown Selection Strategy.");
  }
}

/// @brief  Reviewing the list of submissions based on the user-defined policy
///
/// Check whether the selection_policy passes, if so, and a
/// random draw from U(0, 1) is true, then the submission will
/// be accepted; otherwise, it will be rejected
///
bool PolicyBasedSelection::review(const std::vector<Submission> &subs) {

  /// Checks selection policy returns anything
  /// @todo I don't really like the const_cast here, maybe I need to find a way
  /// to remove it!
  auto check = selection_policy(const_cast<std::vector<Submission> &>(subs));
  if (check and Random::get<bool>(params.acceptance_rate) and
      Random::get<bool>(1 - params.pub_bias))
    return true;

  return false;
}

///
/// Check if `p-value` of the Submission is less than the specified
/// \f$\alpha\f$. If true, it will accept the submission, if not, a random
/// number decide whether the submission is going to be accepted. The
/// drawn random number, \f$r\f$ will be compared to `pub_bias` of the journal.
///
/// @param      s     A reference to the Submission
/// @return     a boolean indicating whether the Submission is accepted.
///
bool SignificantSelection::review(const std::vector<Submission> &subs) {

  // Only accepting +/- results if journal cares about it, side != 0
  if (params.side != 0 and std::any_of(subs.begin(), subs.end(), [&](auto &s) {
        return s.dv_.eff_side_ != params.side;
      })) {
    return false;
  }

  /// Checking whether any of the outcomes are significant
  return std::any_of(subs.begin(), subs.end(),
                     [&](auto &s) -> bool {
                       return s.dv_.pvalue_ < params.alpha;
                     }) or
         Random::get<bool>(1 - params.pub_bias);
}

///
/// Draw a random number between \f$ r \in [0, 1] \f$, reject the submission if
/// \f$ r < 0.5 \f$.
///
/// @param      s     corresponding submission
/// @return     a boolean indicating whether the Submission is accepted.
///
bool RandomSelection::review(const std::vector<Submission> &subs) {
  return Random::get<bool>(0.5);
}