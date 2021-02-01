//===-- ReviewStrategy.h - Review Strategy Deceleration -------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-01-25.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the deceleration of Review Strategy abstract class, and
/// some of its derivatives, e.g., FreeSelection, RandomSelection.
///
//===----------------------------------------------------------------------===//
///
/// @defgroup   ReviewStrategies Review Strategies
///
/// @defgroup   ReviewStrategiesParameters Parameters of Review Strategies

#ifndef SAMPP_REVIEWSTRATEGY_H
#define SAMPP_REVIEWSTRATEGY_H

#include <sol/sol.hpp>

#include "DataStrategy.h"
#include "Distributions.h"
#include "Policy.h"
#include "Submission.h"

namespace sam {

class Journal;

///
/// @brief      List of available review strategies
///
enum class SelectionMethod {
  PolicyBasedSelection,
  SignificantSelection,
  RandomSelection,
  FreeSelection
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    SelectionMethod,
    {{SelectionMethod::PolicyBasedSelection, "PolicyBasedSelection"},
     {SelectionMethod::SignificantSelection, "SignificantSelection"},
     {SelectionMethod::RandomSelection, "RandomSelection"},
     {SelectionMethod::FreeSelection, "FreeSelection"}})

///
/// @brief      Abstract class for Journal's selection strategies.
///
///             A Journal will decide if a Submission is going to be accepted or
///             rejected. This decision can be made based on different criteria
///             or formula. A ReviewStrategy provides an interface for
///             implementing different selection strategies.
///
/// @ingroup    ReviewStrategies
///
class ReviewStrategy {

 public:
  ///
  /// @brief      Pure destructors of the base class
  ///
  virtual ~ReviewStrategy() = 0;

  //! Lua's state
  sol::state lua;

  //! Selection method's name
  SelectionMethod name{};

  /// Constructs the Review Strategy
  ReviewStrategy();

  /// Factory method for building a ReviewStrategy
  static std::unique_ptr<ReviewStrategy> build(json &selection_strategy_config);

  ///
  /// @brief      The abstract _review_ method
  ///
  ///             It reviews the submissions and decides if it's going to be
  ///             accepted or rejected. When deriving from ReviewStrategy,
  ///             `review` is the main interface and `Journal` relies on its
  ///             output
  ///
  /// @param[in]  s     A reference to a Submission
  ///
  /// @return     Returns 'true' if the any of the submissions are going to be
  ///             accepted.
  ///
  virtual bool review(const std::vector<Submission> &s) = 0;

  ///
  /// @brief      The abstract _review_ method.
  ///
  ///             Like the method above, but this one reviews the entire
  ///             Experiment instead.
  ///
  /// @note       It's not yet been used anywhere though SAM, but it will
  ///             eventually replace the submission review method.
  ///
  /// @param[in]  expr  The experiment
  ///
  /// @return     Returns `true` if the Experiment is going to be accepted.
  ///
  virtual bool review(const Experiment &expr) = 0;
};

///
/// @brief      Policy-based Selection Strategy
///
///             Policy-based review strategy accepts a submission if any of the
///             submissions pass all the criteria specified by the
///             `selection_policy_defs`. In addition to the output of selection
///             policy, a submission might get rejected based on journal's
///             acceptance rate, and publication bias rate.
///
/// @note       This is a very flexible method, and it can technically be used
///             to setup wide variety of review strategies, including
///             traditional publication biased journal.
///
/// @ingroup    ReviewStrategies
///
class PolicyBasedSelection final : public ReviewStrategy {
 public:

  ///
  /// @brief      Parameters of the Policy-based Selection
  /// 
  /// @ingroup    ReviewStrategiesParameters
  ///
  struct Parameters {
    SelectionMethod name = SelectionMethod::PolicyBasedSelection;

    //! Publication Bias Rate
    double pub_bias_rate{};

    //! Acceptance Rate
    double acceptance_rate{};

    //! Definition of the selection policy used by Journal to evaluate a given
    //! submission
    std::vector<std::string> selection_policy_defs{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PolicyBasedSelection::Parameters, name,
                                   pub_bias_rate, acceptance_rate,
                                   selection_policy_defs);
  };

  Parameters params;
  PolicyChain selection_policy;

  explicit PolicyBasedSelection(Parameters &p) : params{p} {
    selection_policy = PolicyChain(params.selection_policy_defs,
                                   PolicyChainType::Decision, lua);
  }

  bool review(const std::vector<Submission> &s) override;

  bool review(const Experiment &expr) override { return false; }
};

///
/// @brief      Significant-based Selection Strategy
///
///             Significant-based review strategy accepts a publication if the
///             given *p*-value is significant. Certain degree of *publication
///             bias*, can be specified. In this case, a Submission has a chance
///             of being published even if the statistics is not significant.
///             Moreover, the SignificantSelection can be tailored toward either
///             positive or negative effect. In this case, the Journal will only
///             accept Submissions with larger or smaller effects.
///
/// @ingroup    ReviewStrategies
///
class SignificantSelection final : public ReviewStrategy {
 public:
  ///
  /// A type keeping the parameters of the Significant Selection strategy.
  ///
  /// @ingroup    ReviewStrategiesParameters
  /// 
  struct Parameters {
    //! Selection strategy name
    SelectionMethod name = SelectionMethod::SignificantSelection;

    //! The \alpha at which the _review strategy_ decides the significance
    //! of a publication
    double alpha{};

    //! Publication bias rate
    double pub_bias_rate{};

    //! Indicates the _selection strategy_'s preference toward positive, `1`,
    //! or negative, `-1` effect. If `0`, Journal doesn't have any preferences.
    int side{1};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SignificantSelection::Parameters, name,
                                   alpha, pub_bias_rate, side);
  };

  Parameters params;

  explicit SignificantSelection(const Parameters &p) : params{p} {};

  bool review(const std::vector<Submission> &s) override;

  bool review(const Experiment &expr) override { return false; }
};

///
/// @brief      Random Selection Strategy
///
///             In this method, Journal does not check any criteria for
///             accepting or rejecting a submission. The `acceptance_rate` will
///             decide the acceptance or rejection of a list of submissions.
///
/// @ingroup    ReviewStrategies
///
class RandomSelection final : public ReviewStrategy {
 public:

  ///
  /// @brief      Parameter of _Random Selection_ review strategy
  /// 
  /// @ingroup    ReviewStrategiesParameters
  ///
  struct Parameters {
    SelectionMethod name = SelectionMethod::RandomSelection;

    //! Indicates the acceptance rate of the Journal
    double acceptance_rate{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RandomSelection::Parameters, name, acceptance_rate);
  };

  Parameters params;

  explicit RandomSelection(const Parameters &p) : params{p} {};

  bool review(const std::vector<Submission> &s) override;

  bool review(const Experiment &expr) override { return false; }
};

///
/// @brief      FreeSelection doesn't pose any restriction on the submission and
///             all submissions will be accepted.
///
///
/// @ingroup  ReviewStrategies
///
class FreeSelection final : public ReviewStrategy {
 public:

  ///
  /// @brief      Parameters of free selection
  ///
  ///             Free selection doesn't have any parameters, but this is here
  ///             for consistency.
  ///
  /// @ingroup    ReviewStrategiesParameters
  ///
  struct Parameters {};

  Parameters params;

  FreeSelection()= default;;

  /// It accepts everything
  bool review(const std::vector<Submission> &s) override {
    return true;
  };

  /// It accepts everything
  bool review(const Experiment &expr) override { return true; }
};

}  // namespace sam

#endif  // SAMPP_REVIEWSTRATEGY_H
