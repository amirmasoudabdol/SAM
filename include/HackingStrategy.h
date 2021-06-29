//===-- HackingStrategy.h - Hacking Strategy Deceleration -----------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-01-25.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the deceleration of the Abstract Hacking Strategy class
/// and all the other classes inhereted from it.
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

#include "Distributions.h"
#include "ResearchStrategy.h"
#include "Experiment.h"
#include "HackingStrategyTypes.h"

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

  void operator()(Experiment *experiment) {
    perform(experiment);
  };
  
  [[nodiscard]] float defensibility() const {
    return defensibility_.value();
  }

  [[nodiscard]] float prevalence() const {
    return prevalence_.value();
  }

  [[nodiscard]] HackingStage stage() const {
    return stage_;
  }

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
  /// Parameter of optional stopping method.
  ///
  /// @ingroup  HackingStrategiesParameters
  ///
  struct Parameters {
    //! Placeholder for hacking strategy name
    HackingMethod name = HackingMethod::OptionalStopping;

    //! Number of new observations to be added to each group
    Parameter<int> num;
    
    //! Indicates which groups are going to be targets
    HackingTarget target {HackingTarget::Both};
    
    //! If not 0., `ratio` * n_obs will be added to the experiment.
    Parameter<float> ratio;

    //! Number of times that Researcher add `num` observations to each group
    Parameter<int> n_attempts {1};
    
    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs;
    
    //! The defensibility factor of the strategy
    std::optional<float> defensibility;
    
    //! The prevalence factor of the strategy
    std::optional<float> prevalence;

    //! Execution stage of the strategy
    HackingStage stage {HackingStage::PostProcessing};
    
  };

  Parameters params;
  PolicyChain stopping_condition;

  OptionalStopping() = default;

  explicit OptionalStopping(const Parameters &p)
      : params{p} {
        spdlog::debug("Preparing the Optional Stopping...");
        
        stopping_condition = PolicyChain{p.stopping_cond_defs, PolicyChainType::Decision, lua};
        
        prevalence_ = params.prevalence;
        defensibility_ = params.defensibility;
        stage_ = params.stage;
  };
    
  /// Adds `ns[i]` new items to `i`th group
  ///
  /// @param experiment A pointer to an experiment
  /// @param ns An array indicating how many new items should be added to each group
  void addObservations(Experiment *experiment, const arma::Row<int>& ns);

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
    throw std::invalid_argument("Either `num` or `ratio` should be given as input.");
  }

  if (j.contains("target")) {
    j.at("target").get_to(p.target);
  }
  
  p.n_attempts = Parameter<int>(j.at("n_attempts"), 1);

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
static std::pair<int, int>
getTargetBounds(Experiment *experiment, HackingTarget &target) {
  static int s, e;
  switch (target) {
    case HackingTarget::Control: {
      s = 0; e = experiment->setup.nd();
    } break;
    case HackingTarget::Treatment: {
      s = experiment->setup.nd(); e = experiment->setup.ng();
    } break;
    case HackingTarget::Both: {
      s = 0; e = experiment->setup.ng();
    } break;
  }
  return std::make_pair(s, e);
}



///
/// @brief      Declaration of Outlier Removal hacking method based on items'
///             distance from their sample mean.
///
///
/// @ingroup    HackingStrategies
class OutliersRemoval final : public HackingStrategy {
public:
  
  /// Parameters of Outliers Removal Strategy
  ///
  ///  ```json
  /// {
  ///     "name": "OutliersRemoval",
  ///     "level": "dv",
  ///     "min_observations": 10,
  ///     "multipliers": [
  ///         0.5
  ///     ],
  ///     "n_attempts": 1000,
  ///     "num": 1000,
  ///     "order": "random"
  /// }
  /// ```
  ///
  /// @ingroup HackingStrategiesParameters
  struct Parameters {
    HackingMethod name = HackingMethod::OutliersRemoval;

    //! TO BE IMPLEMENTED!
    HackingTarget target {HackingTarget::Both};

    //! Indicates the order where outliers are going to be removed from the
    //! experiment. \li `max first`, removes the biggest outlier first \li
    //! `random`, removes the first outlier first, this is as a random outlier
    //! is being removed
    std::string order {"max first"};

    //! Indicates the number of outliers to be removed in each iteration
    int num;

    //! Indicates the total number of attempts, i.e., _iterations_, to remove
    //! outliers
    int n_attempts {1};

    //! Indicates the minimum number of observations allowed during the process
    int min_observations;

    //! A list of standard deviation multipliers for identifying outliers
    std::vector<float> multipliers;
    
    //! Indicates the side where the outliers should be removed from,
    //!   - side == 0 → |Z| < k
    //!   - side == 1 →   Z > k
    //!   - side == -1 →  Z < k
    int side{0};
    
    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs;
    
    //! The defensibility factor of the strategy
    std::optional<float> defensibility;
    
    //! The prevalence factor of the strategy
    std::optional<float> prevalence;
    
    HackingStage stage {HackingStage::PostProcessing};
    
  };

  Parameters params;
  
  PolicyChain stopping_condition;

  OutliersRemoval() = default;

  explicit OutliersRemoval(const Parameters &p)
      : params{p} {
        stopping_condition = PolicyChain(params.stopping_cond_defs,
                                         PolicyChainType::Decision,
                                         lua);
        
        prevalence_ = params.prevalence;
        defensibility_ = params.defensibility;
        stage_ = params.stage;
        };

  // Submission hackedSubmission;
  void perform(Experiment *experiment) override;

private:
  bool removeOutliers(Experiment *experiment, const int n, const float k, const int side);
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

///
/// @brief      The subjective outlier removal refers to a type of outliers
/// removal
///             where the researcher continuously lowers the threshold of
///             identifying an outlier, `k`, until it finds a significant (or
///             satisfactory) result.
///
/// @ingroup  HackingStrategies
/// @sa       ResearchStrategy
///
class SubjectiveOutlierRemoval final : public HackingStrategy {
public:
  /// @brief SubjectiveOutlierRemoval's parameters.
  ///
  /// These are parameters specific to this hacking strategy. You can set them
  /// either pragmatically when you are constructing a new
  /// SubjectiveOutlierRemoval, e.g., `SubjectiveOutlierRemoval sor{<name>,
  /// {min, max}, ssize};`.
  ///
  /// Or, when you are using `SAMrun` to run your simulation. In this case,
  /// your JSON variable must comply with the name and type of parameters here.
  /// For example, the following JSON defines the default subjective outliers
  /// removal.
  ///
  /// ```json
  /// {
  ///    "name": "SubjectiveOutlierRemoval",
  ///    "range": [2, 4],
  ///    "step_size": 0.1,
  ///    "min_observations": 5
  /// }
  /// ```
  /// @ingroup HackingStrategiesParameters
  struct Parameters {
    //! A placeholder for the name
    HackingMethod name = HackingMethod::SubjectiveOutlierRemoval;
    
    //! @todo TO BE IMPLEMENTED
    HackingTarget target {HackingTarget::Both};

    //! A vector of `{min, max}`, defining the range of `K`.
    std::vector<int> range;

    //! Indicates the step size of walking through K's
    float step_size;

    //! Indicates minimum number of observations
    int min_observations;
    
    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs {{"sig"}};
    
    //! The prevalence factor of the strategy
    std::optional<float> prevalence;
    
    //! The defensibility factor of the strategy
    std::optional<float> defensibility;
    
    HackingStage stage {HackingStage::PostProcessing};
  };

  Parameters params;
  PolicyChain stopping_condition;

  SubjectiveOutlierRemoval() = default;

  explicit SubjectiveOutlierRemoval(const Parameters &p)
      : params(p) {
          stopping_condition = PolicyChain(params.stopping_cond_defs,
                                           PolicyChainType::Decision,
                                           lua);
        
        prevalence_ = params.prevalence;
        defensibility_ = params.defensibility;
        stage_ = params.stage;
        };

private:
  void perform(Experiment *experiment) override;
};

inline void to_json(json &j, const SubjectiveOutlierRemoval::Parameters &p) {
  j = json{{"name", p.name},
           {"range", p.range},
           {"step_size", p.step_size},
           {"min_observations", p.min_observations},
            {"stage", p.stage},
           {"stopping_condition", p.stopping_cond_defs}};

  if (p.prevalence) {
    j["prevalence"] = p.prevalence.value();
  }

  if (p.defensibility) {
    j["defensibility"] = p.defensibility.value();
  }
}

inline void from_json(const json &j, SubjectiveOutlierRemoval::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);
  j.at("range").get_to(p.range);
  j.at("step_size").get_to(p.step_size);
  j.at("min_observations").get_to(p.min_observations);

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

/// Declaration of GroupPooling hacking strategy
///
/// @ingroup  HackingStrategies
///
class GroupPooling final : public HackingStrategy {

public:
  struct Parameters {
    //! Placeholder for hacking strategy name
    HackingMethod name = HackingMethod::GroupPooling;
    
    //! List of paired indices indicating which groups should be pooled together.
    std::vector<std::vector<int>> pooled_conditions;
  };

  Parameters params;

  GroupPooling() = default;

  explicit GroupPooling(const Parameters &p)
      : params{p} {
        spdlog::debug("Preparing the Group Pooling...");
  };

  void perform(Experiment *experiment) override;

private:
  void pool(Experiment *experiment, int r);
  
  /// Goes through all set of group pairs defined in `pooled_conditions`, pooled their
  /// DVs together, and adds them to the Experiment.
  std::vector<DependentVariable> pool(Experiment *experiment, std::vector<int>& conds, int ng);
  
  /// Pools the values of two dependent variables together, and generates a new
  /// DependentVariable object
  DependentVariable pool(Experiment *experiment, std::vector<int>& gs);
};

inline void to_json(json &j, const GroupPooling::Parameters &p) {
  j = json{{"name", p.name},
            {"pooled_conditions", p.pooled_conditions}};
}

inline void from_json(const json &j, GroupPooling::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);

  j.at("pooled_conditions").get_to(p.pooled_conditions);
}

class ConditionDropping : public HackingStrategy {

public:
  struct Parameters {
    HackingMethod name = HackingMethod::ConditionDropping;
  };

  Parameters params;

  explicit ConditionDropping(const Parameters &p)
      : params{p} {
        std::cerr << "To Be Implemented!";
        exit(1);
        };

  ConditionDropping() { params.name = HackingMethod::ConditionDropping; };

private:
  void perform(Experiment *experiment) override {};
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
/// hack its way to significance by aggressively rounding the pvalue and ignoring
/// everything else.
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

    HackingTarget target {HackingTarget::Treatment};
    
    //! Indicates the distance between the pvalue and alpha by which the researcher
    //! considers to round the pvalue to significance
    float threshold;
    
    /// Rounding Method
    /// - diff: Setting the rounded p-value to the difference between pvalue and threshold
    /// - alpha: Setting the rounded p-value to the value of alpha
    ///
    /// @todo I cna possibly add more methods here, e.g.,
    /// - rounding, where I just round the value down
    /// - random_rounding, where I generate a threshold, then round the `pvalue - threshold` value
    std::string rounding_method = "diff";
    
    //! The prevalence factor of the strategy
    std::optional<float> prevalence;
    
    //! The defensibility factor of the strategy
    std::optional<float> defensibility;
    
    HackingStage stage {HackingStage::Reporting};

  };
  
  Parameters params;
  
  QuestionableRounding() = default;
  
  explicit QuestionableRounding(const Parameters &p) : params{p} {
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



/// PeekingOutliersRemoval Hacking Strategy
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
  ///         0.5
  ///     ],
  ///     "n_attempts": 1000,
  ///     "num": 1000,
  ///     "order": "random"
  ///  }
  /// ```
  ///
  /// @ingroup HackingStrategiesParameters
  ///
  struct Parameters {
    HackingMethod name = HackingMethod::PeekingOutliersRemoval;
    
    //! TO BE IMPLEMENTED!
    HackingTarget target {HackingTarget::Both};
    
    //! Indicates the order where outliers are going to be removed from the
    //! experiment. \li `max first`, removes the biggest outlier first \li
    //! `random`, removes the first outlier first, this is as a random outlier
    //! is being removed
    std::string order {"max first"};
    
    //! Indicates the number of outliers to be removed in each iteration
    int num;
    
    //! Indicates the total number of attempts, i.e., _iterations_, to remove
    //! outliers
    int n_attempts {1};
    
    //! Indicates the minimum number of observations allowed during the process
    int min_observations;
    
    //! A list of standard deviation multipliers for identifying outliers
    std::vector<float> multipliers;
    
    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs;
    
    //! Removing if
    std::vector<std::string> whether_to_save_cond_defs;
    
    //! The prevalence factor of the strategy
    std::optional<float> prevalence;
    
    //! The defensibility factor of the strategy
    std::optional<float> defensibility;
    
    HackingStage stage {HackingStage::PostProcessing};
    
  };
  
  Parameters params;
  PolicyChain stopping_condition;
  PolicyChain whether_to_save_condition;
  
  PeekingOutliersRemoval() = default;
  
  explicit PeekingOutliersRemoval(const Parameters &p) : params{p} {
    prevalence_ = params.prevalence;
    defensibility_ = params.defensibility;
    stage_ = params.stage;
  };
  
  void perform(Experiment *experiment) override;
  
private:
  bool removeOutliers(Experiment *experiment, const int n, const float k);
};

inline void to_json(json &j, const PeekingOutliersRemoval::Parameters &p) {
  j = json{{"name", p.name},
    {"target", p.target},
    {"order", p.order},
    {"num", p.num},
    {"n_attempts", p.n_attempts},
    {"min_observations", p.min_observations},
    {"multipliers", p.multipliers},
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

  if (j.contains("prevalence")) {
    p.prevalence = j.at("prevalence");
  }

  if (j.contains("defensibility")) {
    p.defensibility = j.at("defensibility");
  }
  
  if (j.contains("stage"))
    j.at("stage").get_to(p.stage);
  
  if (j.contains("stopping_condition"))
    j.at("stopping_condition").get_to(p.stopping_cond_defs);
}


/// Falsifying Data Hacking Strategy
///
/// @ingroup HackingStrategies
class FalsifyingData final : public HackingStrategy {
  
public:
  
  /// Falsifying Data Parameters
  ///
  /// Example usage:
  /// ```json
  ///  {
  ///    "name": "FalsifyingData",
  ///  }
  /// ```
  ///
  /// @ingroup HackingStrategiesParameters
  ///
  struct Parameters {
    HackingMethod name = HackingMethod::FalsifyingData;

    //! Falsification approach. We've discussed two possible way of doing this
    //!   - perturbation, perturbing a value
    //!   - group swapping, swapping values between groups
    //!   - group switching, moving values between groups
    std::string approach {"perturbation"};


    //! Switching direction
    //!   - control-to-treatment
    //!   - treatment-to-control
    std::string switching_direction {"control-to-treatment"};

    //! Swapping Method
    //!   - random
    //!   - smart
    std::string selection_method {"random"};
    
    //! Indicates which outcome variables are going to be targeted,
    //!   - control
    //!   - treatment
    //!   - both
    HackingTarget target {HackingTarget::Both};
    
    //! Indicates a set of rule that is going to be used to select the target group
    //! @todo To be implemented
//    PolicyChain target_policy;
    
    //! Number of trials
    int n_attempts {1};
    
    //! Number of observations to be perturbed
    size_t num;
    
    //! Distribution of noise
    std::optional<UnivariateDistribution> noise_dist = makeUnivariateDistribution({
      {"dist", "normal_distribution"},
      {"mean", 0},
      {"stddev", 1}
    });
    
    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs;
    
    //! The defensibility factor of the strategy
    std::optional<float> defensibility;

    //! The prevalence factor of the strategy
    std::optional<float> prevalence;
    
    HackingStage stage {HackingStage::PostProcessing};
  };
  
  Parameters params;
  PolicyChain stopping_condition;
  
  FalsifyingData() = default;
  
  explicit FalsifyingData(const Parameters &p) : params{p} {
    stopping_condition = PolicyChain(params.stopping_cond_defs,
                                     PolicyChainType::Decision,
                                     lua);
    
    prevalence_ = params.prevalence;
    defensibility_ = params.defensibility;
    stage_ = params.stage;
  };
  
  void perform(Experiment *experiment) override;
  
private:
  bool perturb(Experiment *experiment);
  bool swapGroups(Experiment *experiment);
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
//    {"noise_dist", p.noise_dist},
    {"stage", p.stage},
    {"stopping_condition", p.stopping_cond_defs}};

  if (p.prevalence) {
    j["prevalence"] = p.prevalence.value();
  }

  if (p.defensibility) {
    j["defensibility"] = p.defensibility.value();
  }
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
  
  if (j.contains("switching_direction")) {
    j.at("switching_direction").get_to(p.switching_direction);
  }

  if (j.contains("selection_method")) {
    j.at("selection_method").get_to(p.selection_method);
  }

  if (j.contains("stage")) {
    j.at("stage").get_to(p.stage);
  }
  
  if (j.contains("noise")) {
    p.noise_dist = makeUnivariateDistribution(j["noise"]);
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
    std::string approach {"generating"};
    
    //! Indicates which outcome variables are going to be targeted,
    //!   - control
    //!   - treatment
    //!   - both
    HackingTarget target {HackingTarget::Both};
    
    //! Indicates a set of rule that is going to be used to select the target group
    //! @todo To be implemented
//    PolicyChain target_policy;
    
    //! Number of trials
    int n_attempts {1};
    
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
    
    HackingStage stage {HackingStage::PostProcessing};
  };
  
  Parameters params;
  PolicyChain stopping_condition;
  
  FabricatingData() = default;
  
  explicit FabricatingData(const Parameters &p) : params{p} {
    stopping_condition = PolicyChain(params.stopping_cond_defs,
                                     PolicyChainType::Decision,
                                     lua);
    
    prevalence_ = params.prevalence;
    defensibility_ = params.defensibility;
    stage_ = params.stage;
  };
  
  void perform(Experiment *experiment) override;
  
private:
  bool generate(Experiment *experiment, const int n);
  bool duplicate(Experiment *experiment, const int n);
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
    //!   - control
    //!   - treatment
    //!   - both
    //! @todo to be implemented
    HackingTarget target {HackingTarget::Both};
    
    //! Indicates a set of rule that is going to be used to select the target group
    //! @todo To be implemented
    //    PolicyChain target_policy;
        
    //! Number of observations to be perturbed
    int batch_size;
    
    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs {"sig"};
    
    //! The defensibility factor of the strategy
    std::optional<float> defensibility;
    
    //! The prevalence factor of the strategy
    std::optional<float> prevalence;
    
    HackingStage stage {HackingStage::DataCollection};
  };
  
  Parameters params;
  PolicyChain stopping_condition;
  
  StoppingDataCollection() = default;
  
  explicit StoppingDataCollection(const Parameters &p) : params{p} {
    stopping_condition = PolicyChain(params.stopping_cond_defs,
                                     PolicyChainType::Decision,
                                     lua);
    
    prevalence_ = params.prevalence;
    defensibility_ = params.defensibility;
    stage_ = params.stage;
  };
  
  void perform(Experiment *experiment) override;
  
//private:
//  bool generate(Experiment *experiment, const int n);
//  bool duplicate(Experiment *experiment, const int n);
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
/// The _Optional Dropping_ algorithm uses the covariant values to split the dataset, and
/// generates new condition groups.
///
/// @ingroup HackingStrategies
class OptionalDropping final : public HackingStrategy {
  
public:
  
  /// Optional Dropping Collection Parameters
  ///
  /// Indicates the indicies of groups that you like to be used for splitting in `pooled`
  /// variable, e.g. [[1]], the first treatment group; and then use the `split_by`
  /// parameters to define the [index, value] of the covariants.
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
    //!   - control
    //!   - treatment
    //!   - both
    //! @todo to be implemented
    HackingTarget target {HackingTarget::Both};
    
    //! List of condition groups to be used for the dropping procedure
    std::vector<std::vector<int>> pooled;
    
    //! Lists of covariants index, and their value pairs, e.g., [[0, 0], [0, 1]],
    //! that is going to be used by the algorithm to split the dependent variables.
    //! In this case, the data will be splitted by the first covariants (level == 0), and
    //! then by the second covariants (level == 1).
    std::vector<std::vector<int>> split_by;
    
    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs {"sig"};
    
    //! The defensibility factor of the strategy
    std::optional<float> defensibility;
    
    //! The prevalence factor of the strategy
    std::optional<float> prevalence;
    
    HackingStage stage {HackingStage::PostProcessing};
    
  };
  
  Parameters params;
  PolicyChain stopping_condition;
  
  OptionalDropping() = default;
  
  explicit OptionalDropping(const Parameters &p) : params{p} {
    stopping_condition = PolicyChain(params.stopping_cond_defs,
                                     PolicyChainType::Decision,
                                     lua);
    
    prevalence_ = params.prevalence;
    defensibility_ = params.defensibility;
    stage_ = params.stage;
  };
  
  void perform(Experiment *experiment) override;
  
private:
  
  std::vector<DependentVariable> split(Experiment *experiment, std::vector<int> &conds, std::vector<int> &by, int ng);
  DependentVariable split(Experiment *experiment, std::vector<int>& gs, arma::uvec cov);
  
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
