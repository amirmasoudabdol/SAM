//===-- HackingStrategy.h - Hacking Strategy Deceleration -----------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-01-25.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the deceleration of the Abstract Hacking Strategy class
/// and all the other classes inherited from it.
///
//===----------------------------------------------------------------------===//
///
/// @defgroup   HackingStrategies Hacking Strategies
/// @brief      List of available hacking strategies
///
/// @defgroup   HackingStrategiesParameters Parameters of Hacking Strategies
/// @brief      Description of hacking strategies parameters
///

#ifndef SAMPP_HACKINGSTRATEGIES_H
#define SAMPP_HACKINGSTRATEGIES_H

#include <map>
#include <optional>
#include <string>
#include <utility>

#include "Distributions.h"
#include "Experiment.h"
#include "HackingStrategyTypes.h"
#include "ResearchStrategy.h"

namespace sam {

/// @ingroup    HackingStrategies
///
/// @brief      Abstract Class of Hacking Strategies.
///
/// Each HackingStrategy should provide a `perform()` method. The `perform()`
/// method will take over a pointer to an Experiment and apply the implemented
/// hacking on it. Researcher decides if this is a pointer to a *fresh* copy of
/// the Experiment or a pointer to a previously "hacked" Experiment.
///
class HackingStrategy {

public:
  sol::state lua;

  std::optional<float> defensibility_;

  std::optional<float> prevalence_;

  HackingStage stage_;

  HackingTarget target_;

  /// @brief      Pure destructor of the Base class. This is important
  /// for proper deconstruction of Derived classes.
  virtual ~HackingStrategy() = 0;

  HackingStrategy();

  /// @brief      Factory method for building a HackingStrategy
  ///
  /// @param      config  A reference to an item of the
  ///             `json['--hacking-strategy']`. Researcher::Builder is
  ///             responsible for passing this object correctly.
  ///
  /// @return     A new HackingStrategy
  static std::unique_ptr<HackingStrategy> build(json &hacking_strategy_config);

  void operator()(Experiment *experiment) { perform(experiment); };

  [[nodiscard]] float defensibility() const { return defensibility_.value(); }

  [[nodiscard]] float prevalence() const { return prevalence_.value(); }

  [[nodiscard]] HackingStage stage() const { return stage_; }

  [[nodiscard]] HackingTarget target() const { return target_; }

private:
  /// @brief  Applies the hacking method on the Experiment.
  ///
  /// @param  experiment    A pointer to an Experiment.
  virtual void perform(Experiment *experiment) = 0;
};

/// Declaration of OptionalStopping hacking strategy
///
/// @ingroup  HackingStrategies
///
class OptionalStopping final : public HackingStrategy {

public:
  /// Parameter of optional stopping strategy.
  ///
  /// For example, the below parameters configures an optional stopping strategy
  /// where the Researcher adds only 10 new observations to the experiment. If
  /// we would like to allow the researcher to try more than once, we can use
  /// the `n_attempts` parameters to adjust the number of attempts.
  ///
  ///  ```json
  /// {
  ///     "name": "OptionalStopping",
  ///     "num": 10
  /// }
  ///
  /// @ingroup  HackingStrategiesParameters
  ///
  struct Parameters {
    //! Placeholder for hacking strategy name
    HackingMethod name = HackingMethod::OptionalStopping;

    //! Number of new observations to be added to each group. It can be a fixed
    //! value or a univariate distribution that is being used to generate a new
    //! value for each experiment.
    Parameter<int> num;

    //! Indicates which outcome variables are going to be targeted,
    //!   \li Both (default)
    //!   \li Control
    //!   \li Treatment
    HackingTarget target{HackingTarget::Both};

    //! If not 0., `ratio` * n_obs will be added to the experiment.
    Parameter<float> ratio;

    //! Number of times that Researcher add `num` observations to each group
    Parameter<int> n_attempts{1};

    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs;

    //! The defensibility factor of the strategy
    std::optional<float> defensibility;

    //! The prevalence factor of the strategy
    std::optional<float> prevalence;

    //! The default execution stage of the strategy
    HackingStage stage{HackingStage::PostProcessing};
  };

  Parameters params;
  PolicyChain stopping_condition;

  OptionalStopping() = default;

  explicit OptionalStopping(const Parameters &p) : params{p} {
    spdlog::debug("Initializing the Optional Stopping strategy...");

    stopping_condition =
        PolicyChain{p.stopping_cond_defs, PolicyChainType::Decision, lua};

    prevalence_ = params.prevalence;
    defensibility_ = params.defensibility;
    stage_ = params.stage;
  };

  /// Adds new observations to each group
  void addObservations(Experiment *experiment, const arma::Row<int> &ns);

private:
  void perform(Experiment *experiment) override;
};

inline void to_json(json &j, const OptionalStopping::Parameters &p) {
  j = json{{"name", p.name},
           {"num", p.num},
           {"n_attempts", p.n_attempts},
           {"target", p.target},
           {"ratio", p.ratio},
           {"stage", p.stage},
           {"stopping_condition", p.stopping_cond_defs}};

  if (p.prevalence) {
    j["prevalence"] = p.prevalence.value();
  }

  if (p.defensibility) {
    j["defensibility"] = p.defensibility.value();
  }
}

inline void from_json(const json &j, OptionalStopping::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);

  if (j.contains("num")) {

    if (j.at("num") < 1.) {
      spdlog::critical("The `num` parameter should be greater than 1.");
      exit(1);
    }

    p.num = Parameter<int>(j.at("num"), 1);
  } else if (j.contains("ratio")) {
    if (j.at("ratio") < 0. or j.at("ratio") > 1.) {
      spdlog::critical("The `ratio` parameter should be between 0 and 1.");
      exit(1);
    }
    p.ratio = Parameter<float>(j.at("ratio"), 1);
  } else {
    throw std::invalid_argument(
        "Either `num` or `ratio` should be given as input.");
  }

  if (j.contains("target")) {
    j.at("target").get_to(p.target);
  }

  if (j.contains("n_attempts")) {
    p.n_attempts = Parameter<int>(j.at("n_attempts"), 1);
  }

  if (j.contains("prevalence")) {
    p.prevalence = j.at("prevalence");
  }

  if (j.contains("defensibility")) {
    p.defensibility = j.at("defensibility");
  }

  if (j.contains("stage")) {
    j.at("stage").get_to(p.stage);
  }

  if (j.contains("stopping_condition")) {
    j.at("stopping_condition").get_to(p.stopping_cond_defs);
  }
}

///
///
/// @param experiment a pointer to the Experiment object
/// @param target
/// @return
static std::pair<int, int> getTargetBounds(Experiment *experiment,
                                           HackingTarget &target) {
  int s{0};
  int e{0};
  switch (target) {
  case HackingTarget::Control: {
    s = 0;
    e = experiment->setup.nd();
  } break;
  case HackingTarget::Treatment: {
    s = experiment->setup.nd();
    e = experiment->setup.ng();
  } break;
  case HackingTarget::Both: {
    s = 0;
    e = experiment->setup.ng();
  } break;
  }
  return std::make_pair(s, e);
}

///
/// @brief      Declaration of Outliers Removal hacking strategy based on items'
///             distance from the sample mean.
///
/// @note       Note that you can use the Outliers Removal hacking strategy to
///             achieve much more elaborate outliers removal by simply defining
///             a `stopping_condition` parameter. For instance, using {{"sig"}}
///             as the stopping condition will simulate a more greedy version of
///             the Outliers Removal knows as Subjective Outliers Removal.
///
/// @ingroup    HackingStrategies
///
class OutliersRemoval final : public HackingStrategy {
public:
  /// Parameters of Outliers Removal Strategy
  ///
  ///
  /// As an example, the below declaration configures an outliers removal
  /// strategy that leads the Researcher to remove the outliers of the dataset
  /// in 3 consecutive attempts, while removing at most 5 observations in each
  /// trial. In addition, the Researcher stops as soon as there are less than 5
  /// observations left in the dataset.
  ///
  ///  ```json
  /// {
  ///     "name": "OutliersRemoval",
  ///     "num": 5,
  ///     "n_attempts": 3,
  ///     "min_observations": 5,
  ///     "multipliers": [
  ///         2.5
  ///     ]
  /// }
  /// ```
  ///
  /// @ingroup HackingStrategiesParameters
  ///
  struct Parameters {
    HackingMethod name = HackingMethod::OutliersRemoval;

    //! Indicates which outcome variables are going to be targeted,
    //!   \li Both (default)
    //!   \li Control
    //!   \li Treatment
    HackingTarget target{HackingTarget::Both};

    //! Indicates the order where outliers are going to be removed from the
    //! experiment.
    //!
    //! \li `max first`, removes the biggest outlier first
    //! \li `random`, removes the first outlier first, this is as a random
    //! outlier is being removed
    std::string order{"max first"};

    //! Indicates the number of outliers to be removed in each iteration
    int num;

    //! Indicates the total number of attempts, i.e., _iterations_, to remove
    //! outliers
    int n_attempts{1};

    //! Indicates the minimum number of observations allowed during the process
    int min_observations;

    //! A list of standard deviation multipliers for identifying outliers
    std::vector<float> multipliers;

    //! Indicates the side where the outliers should be removed from,
    //!   \li side == 0 → |Z| < k
    //!   \li side == 1 →   Z > k
    //!   \li side == -1 →  Z < k
    int side{0};

    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs;

    //! The defensibility factor of the strategy
    std::optional<float> defensibility;

    //! The prevalence factor of the strategy
    std::optional<float> prevalence;

    //! The default execution stage of the strategy
    HackingStage stage{HackingStage::PostProcessing};
  };

  Parameters params;

  PolicyChain stopping_condition;

  OutliersRemoval() = default;

  explicit OutliersRemoval(Parameters p) : params{std::move(p)} {

    spdlog::debug("Initializing the Outliers Removal strategy...");

    stopping_condition =
        PolicyChain(params.stopping_cond_defs, PolicyChainType::Decision, lua);

    prevalence_ = params.prevalence;
    defensibility_ = params.defensibility;
    stage_ = params.stage;
  };

  void perform(Experiment *experiment) override;

  /// Implementation of the outliers removal
  static bool removeOutliers(Experiment *experiment, int n, float k, int side,
                             HackingTarget &target, std::string &order,
                             int min_n);
};

inline void to_json(json &j, const OutliersRemoval::Parameters &p) {
  j = json{{"name", p.name},
           {"target", p.target},
           {"order", p.order},
           {"num", p.num},
           {"n_attempts", p.n_attempts},
           {"min_observations", p.min_observations},
           {"multipliers", p.multipliers},
           {"side", p.side},
           {"stage", p.stage},
           {"stopping_condition", p.stopping_cond_defs}};

  if (p.prevalence) {
    j["prevalence"] = p.prevalence.value();
  }

  if (p.defensibility) {
    j["defensibility"] = p.defensibility.value();
  }
}

inline void from_json(const json &j, OutliersRemoval::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);

  if (j.contains("target")) {
    j.at("target").get_to(p.target);
  }

  j.at("order").get_to(p.order);
  j.at("num").get_to(p.num);
  j.at("n_attempts").get_to(p.n_attempts);
  j.at("min_observations").get_to(p.min_observations);
  j.at("multipliers").get_to(p.multipliers);
  if (j.contains("side")) {
    j.at("side").get_to(p.side);
  }

  if (j.contains("prevalence")) {
    p.prevalence = j.at("prevalence");
  }

  if (j.contains("defensibility")) {
    p.defensibility = j.at("defensibility");
  }

  if (j.contains("stage")) {
    j.at("stage").get_to(p.stage);
  }

  if (j.contains("stopping_condition")) {
    j.at("stopping_condition").get_to(p.stopping_cond_defs);
  }
}

/// Declaration of GroupPooling hacking strategy.
///
/// This strategy pools two or more treatments groups together, and creates a
/// new treatment groups. In this process, the data from related DVs will be
/// pooled to create a twice as big of a DV with the same property.
///
/// @note       It is even possible to pool a control group into the treatment
///             group to create a new group!
///
/// @ingroup    HackingStrategies
///
class GroupPooling final : public HackingStrategy {

public:
  ///
  /// @brief      Parameters of the Group Pooling hacking strategy
  ///
  struct Parameters {
    //! Placeholder for hacking strategy name
    HackingMethod name = HackingMethod::GroupPooling;

    //! List of paired indices indicating which groups should be pooled
    //! together, e.g., [[1, 2], [1, 3], [1, 2, 3]]. The stopping condition will
    //! be checked between each pair, and the pooling will stop as soon as the
    //! condition is met.
    std::vector<std::vector<int>> pooled_conditions;

    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs;
  };

  Parameters params;
  PolicyChain stopping_condition;

  GroupPooling() = default;

  explicit GroupPooling(Parameters p) : params{std::move(p)} {

    spdlog::debug("Initializing the Group Pooling strategy...");

    stopping_condition =
        PolicyChain(params.stopping_cond_defs, PolicyChainType::Decision, lua);
  };

  void perform(Experiment *experiment) override;

private:
  /// Pools two conditions together, and returns all the pooled dvs
  std::vector<DependentVariable> pool(Experiment *experiment,
                                      std::vector<int> &conds_inx, int ng);

  /// Pools the data from two pained DVs together, and returns a new DV
  DependentVariable pool(Experiment *experiment, std::vector<int> &dvs_inx);
};

inline void to_json(json &j, const GroupPooling::Parameters &p) {
  j = json{{"name", p.name},
           {"pooled_conditions", p.pooled_conditions},
           {"stopping_condition", p.stopping_cond_defs}};
}

inline void from_json(const json &j, GroupPooling::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);

  j.at("pooled_conditions").get_to(p.pooled_conditions);

  if (j.contains("stopping_condition")) {
    j.at("stopping_condition").get_to(p.stopping_cond_defs);
  }
}

class ConditionDropping : public HackingStrategy {

public:
  struct Parameters {
    HackingMethod name = HackingMethod::ConditionDropping;
  };

  Parameters params;

  explicit ConditionDropping(const Parameters &p) : params{p} {
    std::cerr << "To Be Implemented!";
    exit(1);
  };

  ConditionDropping() { params.name = HackingMethod::ConditionDropping; };

private:
  void perform(Experiment *experiment) override{};
};

inline void to_json(json &j, const ConditionDropping::Parameters &p) {
  j = json{{"name", p.name}};
}

inline void from_json(const json &j, ConditionDropping::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);
}

/// QuestionableRounding Hacking Strategy
///
/// Questionable rounding strategy mimics the behavior of a researcher who might
/// hack its way to significance by aggressively rounding the p-value and
/// ignoring everything else.
///
/// @ingroup HackingStrategies
class QuestionableRounding final : public HackingStrategy {

public:
  /// Questionable Rounding Parameters
  ///
  /// Example usage:
  /// ```json
  ///  {
  ///    "name": "QuestionableRounding",
  ///    "rounding_method": "alpha",
  ///    "threshold": 0.01,
  ///    "prevalence": 0.1,
  ///    "defensibility": 0.9,
  ///    "stage": "Reporting"
  ///  }
  /// ```
  ///
  /// @ingroup HackingStrategiesParameters
  ///
  struct Parameters {
    HackingMethod name = HackingMethod::QuestionableRounding;

    //! Indicates which outcome variables are going to be targeted,
    //!   \li Both (default)
    //!   \li Control
    //!   \li Treatment
    HackingTarget target{HackingTarget::Treatment};

    //! Indicates the distance between the p-value and alpha by which the
    //! researcher considers to round the p-value to significance
    float threshold;

    /// Rounding Method
    /// \li diff: Setting the rounded p-value to the difference between p-value
    /// and threshold
    /// \li alpha: Setting the rounded p-value to the value of alpha
    ///
    /// @todo I can possibly add more methods here, e.g.,
    /// \li rounding, where I just round the value down
    /// \li random_rounding, where I generate a threshold, then round the
    /// `p-value \li threshold` value
    std::string rounding_method = "diff";

    //! The prevalence factor of the strategy
    std::optional<float> prevalence;

    //! The defensibility factor of the strategy
    std::optional<float> defensibility;

    //! The default execution stage of the strategy
    HackingStage stage{HackingStage::Reporting};
  };

  Parameters params;

  QuestionableRounding() = default;

  explicit QuestionableRounding(Parameters p) : params{std::move(p)} {

    spdlog::debug("Initializing the Questionable Rounding strategy...");

    prevalence_ = params.prevalence;
    defensibility_ = params.defensibility;
    stage_ = params.stage;
  };

  void perform(Experiment *experiment) override;
};

inline void to_json(json &j, const QuestionableRounding::Parameters &p) {
  j = json{{"name", p.name},
           {"threshold", p.threshold},
           {"rounding_method", p.rounding_method},
           {"stage", p.stage}};

  if (p.prevalence) {
    j["prevalence"] = p.prevalence.value();
  }

  if (p.defensibility) {
    j["defensibility"] = p.defensibility.value();
  }
}

inline void from_json(const json &j, QuestionableRounding::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);
  j.at("threshold").get_to(p.threshold);
  j.at("rounding_method").get_to(p.rounding_method);
  j.at("stage").get_to(p.stage);

  if (j.contains("prevalence")) {
    p.prevalence = j.at("prevalence");
  }

  if (j.contains("defensibility")) {
    p.defensibility = j.at("defensibility");
  }
}

/// @brief Declaration of the Peeking Outliers Removal Hacking Strategy
///
/// In this strategy, the Researcher performs the outliers removal on the copy
/// of the Experiment to peek into its effect. If the result is satisfactory,
/// she then saves the altered dataset, and registers the outliers removal.
///
/// @ingroup HackingStrategies
class PeekingOutliersRemoval final : public HackingStrategy {

public:
  /// Peaking Outliers Removal Parameters
  ///
  /// Example usage:
  /// ```json
  ///  {
  ///    "name": "PeekingOutliersRemoval",
  ///     "level": "dv",
  ///     "min_observations": 10,
  ///     "multipliers": [
  ///         2.5
  ///     ],
  ///     "n_attempts": 2,
  ///     "num": 5,
  ///     "order": "random"
  ///  }
  /// ```
  ///
  /// @ingroup HackingStrategiesParameters
  ///
  struct Parameters {
    HackingMethod name = HackingMethod::PeekingOutliersRemoval;

    //! Indicates which outcome variables are going to be targeted,
    //!   \li Both (default)
    //!   \li Control
    //!   \li Treatment
    HackingTarget target{HackingTarget::Both};

    //! Indicates the order where outliers are going to be removed from the
    //! experiment.
    //!
    //! \li `max first`, removes the biggest outlier first
    //! \li `random`, removes the first outlier first, this is as a random
    //! outlier is being removed
    std::string order{"max first"};

    //! Indicates the number of outliers to be removed in each iteration
    int num;

    //! Indicates the total number of attempts, i.e., _iterations_, to remove
    //! outliers
    int n_attempts{1};

    //! Indicates the minimum number of observations allowed during the process
    int min_observations;

    //! A list of standard deviation multipliers for identifying outliers
    std::vector<float> multipliers;

    //! Indicates the side where the outliers should be removed from,
    //!   \li side == 0 → |Z| < k
    //!   \li side == 1 →   Z > k
    //!   \li side == -1 →  Z < k
    int side{0};

    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs;

    //! A decision policy used to evaluate the status of the experiment after
    //! performing the outliers removal. The Researcher only commits to the
    //! outliers removal only if this condition passes, otherwise, she will
    //! ignore the step, and the peeking will be unsuccessful.
    std::vector<std::string> whether_to_save_cond_defs;

    //! The prevalence factor of the strategy
    std::optional<float> prevalence;

    //! The defensibility factor of the strategy
    std::optional<float> defensibility;

    //! The default execution stage of the strategy
    HackingStage stage{HackingStage::PostProcessing};
  };

  Parameters params;
  PolicyChain stopping_condition;
  PolicyChain whether_to_save_condition;

  PeekingOutliersRemoval() = default;

  explicit PeekingOutliersRemoval(Parameters p) : params{std::move(p)} {

    spdlog::debug("Initializing the Peeking Outliers Removal strategy...");

    prevalence_ = params.prevalence;
    defensibility_ = params.defensibility;
    stage_ = params.stage;
  };

  void perform(Experiment *experiment) override;
};

inline void to_json(json &j, const PeekingOutliersRemoval::Parameters &p) {
  j = json{{"name", p.name},
           {"target", p.target},
           {"order", p.order},
           {"num", p.num},
           {"n_attempts", p.n_attempts},
           {"min_observations", p.min_observations},
           {"multipliers", p.multipliers},
           {"side", p.side},
           {"stage", p.stage},
           {"whether_to_save_condition", p.whether_to_save_cond_defs},
           {"stopping_condition", p.stopping_cond_defs}};

  if (p.prevalence) {
    j["prevalence"] = p.prevalence.value();
  }

  if (p.defensibility) {
    j["defensibility"] = p.defensibility.value();
  }
}

inline void from_json(const json &j, PeekingOutliersRemoval::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);

  if (j.contains("target")) {
    j.at("target").get_to(p.target);
  }

  j.at("order").get_to(p.order);
  j.at("num").get_to(p.num);
  j.at("n_attempts").get_to(p.n_attempts);
  j.at("min_observations").get_to(p.min_observations);
  j.at("multipliers").get_to(p.multipliers);
  j.at("whether_to_save_condition").get_to(p.whether_to_save_cond_defs);

  if (j.contains("side")) {
    j.at("side").get_to(p.side);
  }

  if (j.contains("prevalence")) {
    p.prevalence = j.at("prevalence");
  }

  if (j.contains("defensibility")) {
    p.defensibility = j.at("defensibility");
  }

  if (j.contains("stage")) {
    j.at("stage").get_to(p.stage);
  }

  if (j.contains("stopping_condition")) {
    j.at("stopping_condition").get_to(p.stopping_cond_defs);
  }
}

/// Falsifying Data Hacking Strategy
///
/// This algorithm falsifies the available datasets using different means, e.g.,
/// *perturbation*, *swapping* or *switching*.
///
/// \li Perturbation, a selected number of data points will be perturbed by
/// adding some noise to them.
/// \li Group Swapping, a selected number of data points will be swapped between
/// two groups.
/// \li Group Switching, a selected number of data points will be moved from
/// one group to another.
///
/// @ingroup HackingStrategies
///
class FalsifyingData final : public HackingStrategy {

public:
  /// Falsifying Data Parameters
  ///
  /// Example usage:
  /// ```json
  ///  {
  ///    "name": "FalsifyingData",
  ///    "approach": "perturbation",
  ///    "num": 5,
  ///  }
  /// ```
  ///
  /// @ingroup HackingStrategiesParameters
  ///
  struct Parameters {
    HackingMethod name = HackingMethod::FalsifyingData;

    //! Falsification approach. We've discussed two possible way of doing this
    //!   \li Perturbation, perturbing a value
    //!   \li Group Swapping, swapping values between groups
    //!   \li Group Switching, moving values between groups
    std::string approach{"perturbation"};

    //! Switching direction
    //!   \li  control-to-treatment
    //!   \li  treatment-to-control
    std::string switching_direction{"control-to-treatment"};

    //! Swapping Method
    //!   \li random
    //!   \li smart, selecting the
    std::string selection_method{"random"};

    //! Indicates which outcome variables are going to be targeted,
    //!   \li Both (default)
    //!   \li Control
    //!   \li Treatment
    HackingTarget target{HackingTarget::Both};

    //! Indicates a set of rule that is going to be used to select the target
    //! group
    //! @todo To be implemented, @maybe
    //    PolicyChain target_policy;

    //! Number of trials
    int n_attempts{1};

    //! Number of observations that are going to be affected/used
    size_t num;

    //! Distribution of noise. Default: e ~ N(0, 1)
    std::optional<UnivariateDistribution> noise = makeUnivariateDistribution(
        {{"dist", "normal_distribution"}, {"mean", 0}, {"stddev", 1}});

    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs;

    //! The defensibility factor of the strategy
    std::optional<float> defensibility;

    //! The prevalence factor of the strategy
    std::optional<float> prevalence;

    //! The default execution stage of the strategy
    HackingStage stage{HackingStage::PostProcessing};
  };

  Parameters params;
  PolicyChain stopping_condition;

  FalsifyingData() = default;

  explicit FalsifyingData(Parameters p) : params{std::move(p)} {

    spdlog::debug("Initializing the Falsifying Data strategy...");

    stopping_condition =
        PolicyChain(params.stopping_cond_defs, PolicyChainType::Decision, lua);

    prevalence_ = params.prevalence;
    defensibility_ = params.defensibility;
    stage_ = params.stage;
  };

  void perform(Experiment *experiment) override;

private:
  /// Perturbs a set of observations by adding noise to them
  bool perturb(Experiment *experiment);

  /// Swaps a set of observations between two groups
  bool swapGroups(Experiment *experiment);

  /// Switches a set of observations from one group to another
  bool switchGroups(Experiment *experiment);
};

inline void to_json(json &j, const FalsifyingData::Parameters &p) {
  j = json{{"name", p.name},
           {"approach", p.approach},
           {"n_attempts", p.n_attempts},
           {"num", p.num},
           {"target", p.target},
           {"switching_direction", p.switching_direction},
           {"selection_method", p.selection_method},
           {"stage", p.stage},
           {"stopping_condition", p.stopping_cond_defs}};

  if (p.prevalence) {
    j["prevalence"] = p.prevalence.value();
  }

  if (p.defensibility) {
    j["defensibility"] = p.defensibility.value();
  }

  // @todo this is not possible at the moment, but it'd be nice to have it
  // j["noise"] = p.noise.value();
}

inline void from_json(const json &j, FalsifyingData::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);

  j.at("approach").get_to(p.approach);
  j.at("n_attempts").get_to(p.n_attempts);
  j.at("num").get_to(p.num);

  if (j.contains("target")) {
    j.at("target").get_to(p.target);
  }

  if (j.contains("prevalence")) {
    p.prevalence = j.at("prevalence");
  }

  if (j.contains("defensibility")) {
    p.defensibility = j.at("defensibility");
  }

  if (j["approach"] == "switching" and
      (not j.contains("switching_direction"))) {
    j.at("switching_direction").get_to(p.switching_direction);
  } else {
    spdlog::critical("The switching direction is not defined, see \
      `switching_direction` parameter.");
    exit(1);
  }

  if ((j["approach"] == "switching" or j["approach"] == "swapping") and
      (not j.contains("selection_method"))) {
    j.at("selection_method").get_to(p.selection_method);
  } else {
    spdlog::critical("The selection method is not defined, see \
      `selection_method` parameter.");
  }

  if (j.contains("stage")) {
    j.at("stage").get_to(p.stage);
  }

  if (j.contains("noise")) {
    p.noise = makeUnivariateDistribution(j["noise"]);
  }

  if (j.contains("stopping_condition")) {
    j.at("stopping_condition").get_to(p.stopping_cond_defs);
  }
}

/// Fabricating Data Hacking Strategy
///
/// @ingroup HackingStrategies
class FabricatingData final : public HackingStrategy {

public:
  /// Fabricating Data Parameters
  ///
  /// Example usage:
  /// ```json
  ///  {
  ///    "name": "FabricatingData",
  ///  }
  /// ```
  ///
  /// @ingroup HackingStrategiesParameters
  ///
  struct Parameters {
    HackingMethod name = HackingMethod::FabricatingData;

    //! Falsification approach. We've discussed two possible way of doing this
    //!   - generating, perturbing a value
    //!   - duplicating, swapping values between groups
    std::string approach{"generating"};

    //! Indicates which outcome variables are going to be targeted,
    //!   \li Both (default)
    //!   \li Control
    //!   \li Treatment
    HackingTarget target{HackingTarget::Both};

    //! Indicates a set of rule that is going to be used to select the target
    //! group
    //! @todo To be implemented
    //    PolicyChain target_policy;

    //! Number of trials
    int n_attempts{1};

    //! Number of observations to be perturbed
    int num;

    //! Distribution of fabricated data
    //! @todo Check if this is even necessary, I think in most cases, we
    //! can probably just use the data_strategy and get over it
    std::optional<UnivariateDistribution> dist;

    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs;

    //! The defensibility factor of the strategy
    std::optional<float> defensibility;

    //! The prevalence factor of the strategy
    std::optional<float> prevalence;

    //! The default execution stage of the strategy
    HackingStage stage{HackingStage::PostProcessing};
  };

  Parameters params;
  PolicyChain stopping_condition;

  FabricatingData() = default;

  explicit FabricatingData(Parameters p) : params{std::move(p)} {

    spdlog::debug("Initializing the Fabricating Data strategy...");

    stopping_condition =
        PolicyChain(params.stopping_cond_defs, PolicyChainType::Decision, lua);

    prevalence_ = params.prevalence;
    defensibility_ = params.defensibility;
    stage_ = params.stage;
  };

  void perform(Experiment *experiment) override;

private:
  bool generate(Experiment *experiment, int n);
  bool duplicate(Experiment *experiment, int n);
};

inline void to_json(json &j, const FabricatingData::Parameters &p) {
  j = json{{"name", p.name},
           {"approach", p.approach},
           {"n_attempts", p.n_attempts},
           {"num", p.num},
           {"target", p.target},
           //    {"dist", p.dist},
           {"stage", p.stage},
           {"stopping_condition", p.stopping_cond_defs}};

  if (p.prevalence) {
    j["prevalence"] = p.prevalence.value();
  }

  if (p.defensibility) {
    j["defensibility"] = p.defensibility.value();
  }
}

inline void from_json(const json &j, FabricatingData::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);

  j.at("approach").get_to(p.approach);
  j.at("n_attempts").get_to(p.n_attempts);
  j.at("num").get_to(p.num);

  if (j.contains("target")) {
    j.at("target").get_to(p.target);
  }

  if (j.contains("prevalence")) {
    p.prevalence = j.at("prevalence");
  }

  if (j.contains("defensibility")) {
    p.defensibility = j.at("defensibility");
  }

  if (j.contains("stage")) {
    j.at("stage").get_to(p.stage);
  }

  if (j.contains("dist")) {
    p.dist = makeUnivariateDistribution(j["dist"]);
  }

  if (j.contains("stopping_condition")) {
    j.at("stopping_condition").get_to(p.stopping_cond_defs);
  }
}

/// Fabricating Data Hacking Strategy
///
/// @ingroup HackingStrategies
class StoppingDataCollection final : public HackingStrategy {

public:
  /// Stopping Data Collection Parameters
  ///
  /// Example usage:
  /// ```json
  ///  {
  ///    "name": "StoppingDataCollection",
  ///    "batch_size": 5,
  ///    "stopping_condition": ["sig"]
  ///  }
  /// ```
  ///
  /// @ingroup HackingStrategiesParameters
  ///
  struct Parameters {
    HackingMethod name = HackingMethod::StoppingDataCollection;

    //! Indicates which outcome variables are going to be targeted,
    //!   \li Both (default)
    //!   \li Control
    //!   \li Treatment
    //! @todo to be implemented
    HackingTarget target{HackingTarget::Both};

    //! Indicates a set of rule that is going to be used to select the target
    //! group
    //! @todo To be implemented
    //    PolicyChain target_policy;

    //! Number of observations to be perturbed
    int batch_size;

    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs{"sig"};

    //! The defensibility factor of the strategy
    std::optional<float> defensibility;

    //! The prevalence factor of the strategy
    std::optional<float> prevalence;

    //! The default execution stage of the strategy
    HackingStage stage{HackingStage::DataCollection};
  };

  Parameters params;
  PolicyChain stopping_condition;

  StoppingDataCollection() = default;

  explicit StoppingDataCollection(Parameters p) : params{std::move(p)} {

    spdlog::debug("Initializing the Stopping Data Collection strategy...");

    stopping_condition =
        PolicyChain(params.stopping_cond_defs, PolicyChainType::Decision, lua);

    prevalence_ = params.prevalence;
    defensibility_ = params.defensibility;
    stage_ = params.stage;
  };

  void perform(Experiment *experiment) override;

  // private:
  //   bool generate(Experiment *experiment, const int n);
  //   bool duplicate(Experiment *experiment, const int n);
};

inline void to_json(json &j, const StoppingDataCollection::Parameters &p) {
  j = json{{"name", p.name},
           {"batch_size", p.batch_size},
           {"stage", p.stage},
           {"stopping_condition", p.stopping_cond_defs}};

  if (p.prevalence) {
    j["prevalence"] = p.prevalence.value();
  }

  if (p.defensibility) {
    j["defensibility"] = p.defensibility.value();
  }
}

inline void from_json(const json &j, StoppingDataCollection::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);

  j.at("batch_size").get_to(p.batch_size);

  if (j.contains("prevalence")) {
    p.prevalence = j.at("prevalence");
  }

  if (j.contains("defensibility")) {
    p.defensibility = j.at("defensibility");
  }

  if (j.contains("stage")) {
    j.at("stage").get_to(p.stage);
  }

  if (j.contains("stopping_condition")) {
    j.at("stopping_condition").get_to(p.stopping_cond_defs);
  }
}

/// Optional Dropping Hacking Strategy
///
/// The _Optional Dropping_ algorithm uses the covariant values to split the
/// dataset, and generates new condition groups.
///
/// @ingroup HackingStrategies
class OptionalDropping final : public HackingStrategy {

public:
  /// Optional Dropping Collection Parameters
  ///
  /// Indicates the indices of groups that you like to be used for splitting in
  /// `pooled` variable, e.g. [[1]], the first treatment group; and then use the
  /// `split_by` parameters to define the [index, value] of the covariant.
  ///
  /// Example usage:
  /// ```json
  ///  {
  ///    "name": "OptionalDropping",
  ///    "pooled": [[1]],
  ///    "split_by": [[0, 1]],
  ///    "stopping_condition": ["sig"]
  ///  }
  /// ```
  ///
  /// @ingroup HackingStrategiesParameters
  ///
  struct Parameters {
    HackingMethod name = HackingMethod::OptionalDropping;

    //! Indicates which outcome variables are going to be targeted,
    //!   \li Both (default)
    //!   \li Control
    //!   \li Treatment
    //! @todo to be implemented
    HackingTarget target{HackingTarget::Both};

    //! List of condition groups to be used for the dropping procedure
    std::vector<std::vector<int>> pooled;

    //! Lists of covariant index, and their value pairs, e.g., [[0, 0], [0, 1]],
    //! that is going to be used by the algorithm to split the dependent
    //! variables. In this case, the data will be split by the first covariant
    //! (level == 0), and then by the second covariant (level == 1).
    std::vector<std::vector<int>> split_by;

    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs{"sig"};

    //! The defensibility factor of the strategy
    std::optional<float> defensibility;

    //! The prevalence factor of the strategy
    std::optional<float> prevalence;

    //! The default execution stage of the strategy
    HackingStage stage{HackingStage::PostProcessing};
  };

  Parameters params;
  PolicyChain stopping_condition;

  OptionalDropping() = default;

  explicit OptionalDropping(Parameters p) : params{std::move(p)} {

    spdlog::debug("Initializing the Optional Dropping strategy...");

    stopping_condition =
        PolicyChain(params.stopping_cond_defs, PolicyChainType::Decision, lua);

    prevalence_ = params.prevalence;
    defensibility_ = params.defensibility;
    stage_ = params.stage;
  };

  void perform(Experiment *experiment) override;

private:
  std::vector<DependentVariable> split(Experiment *experiment,
                                       std::vector<int> &conds,
                                       std::vector<int> &by, int ng);
  DependentVariable split(Experiment *experiment, std::vector<int> &gs,
                          arma::uvec cov);
};

inline void to_json(json &j, const OptionalDropping::Parameters &p) {
  j = json{{"name", p.name},
           {"pooled", p.pooled},
           {"split_by", p.split_by},
           {"stage", p.stage},
           {"stopping_condition", p.stopping_cond_defs}};

  if (p.prevalence) {
    j["prevalence"] = p.prevalence.value();
  }

  if (p.defensibility) {
    j["defensibility"] = p.defensibility.value();
  }
}

inline void from_json(const json &j, OptionalDropping::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);
  j.at("pooled").get_to(p.pooled);
  j.at("split_by").get_to(p.split_by);

  if (j.contains("prevalence")) {
    p.prevalence = j.at("prevalence");
  }

  if (j.contains("defensibility")) {
    p.defensibility = j.at("defensibility");
  }

  if (j.contains("stage")) {
    j.at("stage").get_to(p.stage);
  }

  if (j.contains("stopping_condition")) {
    j.at("stopping_condition").get_to(p.stopping_cond_defs);
  }
}

} // namespace sam

#endif // SAMPP_HACKINGSTRATEGIES_H
