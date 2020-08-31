//
// Created by Amir Masoud Abdol on 2019-01-25.
//

///
/// \defgroup   HackingStrategies (group_title)
/// \brief      List of available hacking strategies
///
/// Description to come!
///

///
/// \defgroup   HackingStrategiesParameters (group_title)
/// \brief      Description of hacking strategies parameters
///
/// Description to come!
///

#ifndef SAMPP_HACKINGSTRATEGIES_H
#define SAMPP_HACKINGSTRATEGIES_H

#include <map>
#include <string>

#include "Utilities.h"
#include "DecisionStrategy.h"
#include "Experiment.h"
#include "HackingStrategyTypes.h"

namespace sam {

/// @ingroup    HackingStrategies
///
/// @brief      Abstract class for hacking strategies.
///
/// Each HackingStrategy should provide a `perform()` method. The `perform()`
/// method will take over a pointer to an Experiment and apply the implemented
/// hacking on it. Researcher decides if this is a pointer to a *fresh* copy of
/// the Experiment or a pointer to a previously "hacked" Experiment.
///
class HackingStrategy {

public:
  
  sol::state lua;
  
  /// @brief      Pure deconstuctor of the Base calss. This is important
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

  static std::unique_ptr<HackingStrategy> build(HackingMethod method);

  
  void operator()(Experiment *experiment) {
    perform(experiment);
  };

private:
  /// @brief  Applies the hacking method on the Experiment.
  ///
  /// @param  experiment    A pointer to an Experiment.
  virtual void perform(Experiment *experiment) = 0;
};

class NoHack final : public HackingStrategy {
public:
  NoHack(){

  };

private:
  virtual void perform(Experiment *experiment) override{};
};

/// Declartion of OptionalStopping hacking strategy
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
    int num = 3;
    
    //! Indicates which groups are going to be targets
    std::string target {"both"};
    
    //! If not 0., `add_by_fraction` * n_obs will be added to the experiment.
    double add_by_fraction = 0.;

    //! Number of times that Researcher add `num` observations to each group
    int n_attempts = 1;
    
    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs;
    
  };

  Parameters params;
  PolicyChain stopping_condition;

  OptionalStopping() = default;

  OptionalStopping(const Parameters &p)
      : params{p} {
        stopping_condition = PolicyChain{p.stopping_cond_defs, lua};
  };
  
  /// Adds `n` observations to all groups
  ///
  /// @param experiment A pointer to the experiment
  /// @param n number of new observations to be added
  void addObservations(Experiment *experiment, const int n);
  
  
  /// Adds `ns[i]` new items to `i`th group
  ///
  /// @param experiment A pointer to an experiment
  /// @param ns An array indicating how many new items should be added to each group
  void addObservations(Experiment *experiment, const arma::Row<int> ns);

private:
  virtual void perform(Experiment *experiment) override;
};

inline void to_json(json &j, const OptionalStopping::Parameters &p) {
  j = json{{"name", p.name},
           {"num", p.num},
           {"n_attempts", p.n_attempts},
           {"target", p.target},
           {"add_by_fraction", p.add_by_fraction},
           {"stopping_condition", p.stopping_cond_defs}
  };
}

inline void from_json(const json &j, OptionalStopping::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);
  j.at("num").get_to(p.num);
  j.at("target").get_to(p.target);
  j.at("n_attempts").get_to(p.n_attempts);
  
  if (j.contains("stopping_condition"))
    j.at("stopping_condition").get_to(p.stopping_cond_defs);
  
  if (j.contains("add_by_fraction"))
    j.at("add_by_fraction").get_to(p.add_by_fraction);
  
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
    std::string level = "dv";

    //! Indicates the order where outliers are going to be removed from the
    //! experiment. \li `max first`, removes the biggest outlier first \li
    //! `random`, removes the first outlier first, this is as a random outlier
    //! is being removed
    std::string order = "max first";

    //! Indicates the number of outliers to be removed in each iteration
    int num{3};

    //! Indicates the total number of attempts, i.e., _iterations_, to remove
    //! outliers
    int n_attempts{1};

    //! Indicates the minimum number of observations allowed during the process
    int min_observations{15};

    //! A list of standard deviation multipliers for identidying outliers
    std::vector<double> multipliers = {3};
    
    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs;
  };

  Parameters params;
  
  PolicyChain stopping_condition;

  OutliersRemoval() = default;

  OutliersRemoval(const Parameters &p)
      : params{p} {
        stopping_condition = PolicyChain(params.stopping_cond_defs, lua);
        };

  // Submission hackedSubmission;
  virtual void perform(Experiment *experiment) override;

private:
  bool removeOutliers(Experiment *experiment, const int n, const double k);
};

inline void to_json(json &j, const OutliersRemoval::Parameters &p) {
  j = json{{"name", p.name},
           {"level", p.level},
           {"order", p.order},
           {"num", p.num},
           {"n_attempts", p.n_attempts},
           {"min_observations", p.min_observations},
           {"multipliers", p.multipliers},
           {"stopping_condition", p.stopping_cond_defs}};
}

inline void from_json(const json &j, OutliersRemoval::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);

  j.at("level").get_to(p.level);
  j.at("order").get_to(p.order);
  j.at("num").get_to(p.num);
  j.at("n_attempts").get_to(p.n_attempts);
  j.at("min_observations").get_to(p.min_observations);
  j.at("multipliers").get_to(p.multipliers);
  
  if (j.contains("stopping_condition"))
    j.at("stopping_condition").get_to(p.stopping_cond_defs);
}

///
/// @brief      The subjective outlier removal refers to a type of outliers
/// removal
///             where the researcher continiously lowers the threshold of
///             identifying an outlier, `k`, until it finds a significant (or
///             satisfactory) result.
///
/// @ingroup  HackingStrategies
/// @sa       DecisionStrategy
///
class SubjectiveOutlierRemoval final : public HackingStrategy {
public:
  /// @brief SubjectiveOutlierRemoval's parameters.
  ///
  /// These are parameters specific to this hacking strategy. You can set them
  /// either progmatically when you are constructing a new
  /// SubjectiveOutlierRemoval, e.g., `SubjectiveOutlierRemoval sor{<name>,
  /// {min, max}, ssize};`.
  ///
  /// Or, when you are using `SAMrun` to run your simulation. In this case,
  /// your JSON variable must comply with the name and type of paramters here.
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

    //! A vector of `{min, max}`, defining the range of `K`.
    std::vector<int> range{2, 4};

    //! Indicates the step size of walking through K's
    double step_size{0.1};

    //! Indicates minimum number of observatons
    int min_observations{5};
    
    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs {{"sig"}};
  };

  Parameters params;
  PolicyChain stopping_condition;

  SubjectiveOutlierRemoval() = default;

  SubjectiveOutlierRemoval(const Parameters &p)
      : params(p) {
          stopping_condition = PolicyChain(params.stopping_cond_defs, lua);
        };

private:
  virtual void perform(Experiment *experiment) override;
};

inline void to_json(json &j, const SubjectiveOutlierRemoval::Parameters &p) {
  j = json{{"name", p.name},
           {"range", p.range},
           {"step_size", p.step_size},
           {"min_observations", p.min_observations},
           {"stopping_condition", p.stopping_cond_defs}};
}

inline void from_json(const json &j, SubjectiveOutlierRemoval::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);
  j.at("range").get_to(p.range);
  j.at("step_size").get_to(p.step_size);
  j.at("min_observations").get_to(p.min_observations);
  
  if (j.contains("stopping_condition"))
    j.at("stopping_condition").get_to(p.stopping_cond_defs);
}

class GroupPooling final : public HackingStrategy {

public:
  struct Parameters {
    HackingMethod name = HackingMethod::GroupPooling;
    std::vector<int> nums = {2};
  };

  Parameters params;

  GroupPooling() = default;

  GroupPooling(const Parameters &p)
      : params{p} {
        
        };

  virtual void perform(Experiment *experiment) override;

private:
  void pool(Experiment *experiment, int r);
};

inline void to_json(json &j, const GroupPooling::Parameters &p) {
  j = json{{"name", p.name}, {"nums", p.nums}};
}

inline void from_json(const json &j, GroupPooling::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);

  j.at("nums").get_to(p.nums);
}

class ConditionDropping : public HackingStrategy {

public:
  struct Parameters {
    HackingMethod name = HackingMethod::ConditionDropping;
  };

  Parameters params;

  ConditionDropping(const Parameters &p)
      : params{p} {

        };

  ConditionDropping() { params.name = HackingMethod::ConditionDropping; };

private:
  virtual void perform(Experiment *experiment) override {};
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
/// hack its way to significace by aggressively rounding the pvalue and ignoring
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
  ///    "threshold": 0.01
  ///  }
  /// ```
  ///
  /// @ingroup HackingStrategiesParameters
  ///
  struct Parameters {
    HackingMethod name = HackingMethod::QuestionableRounding;
    
    //! Indicates the distance between the pvalue and alpha by which the researcher
    //! considers to round the pvalue to significance
    double threshold {0.005};
    
    /// Rounding Method
    /// - diff: Setting the rounded p-value to the difference between pvalue and threshold
    /// - alpha: Setting the rounded p-value to the value of alpha
    ///
    /// \todo I cna possibly add more methods here, e.g.,
    /// - rounding, where I just round the value down
    /// - random_rounding, where I generate a threshold, then round the `pvalue - threshold` value
    std::string rounding_method = "diff";
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(QuestionableRounding::Parameters, name, threshold,  rounding_method);
  };
  
  Parameters params;
  
  QuestionableRounding() = default;
  
  QuestionableRounding(const Parameters &p) : params{p} { };
  
  virtual void perform(Experiment *experiment) override;
};


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
    std::string level = "dv";
    
    //! Indicates the order where outliers are going to be removed from the
    //! experiment. \li `max first`, removes the biggest outlier first \li
    //! `random`, removes the first outlier first, this is as a random outlier
    //! is being removed
    std::string order = "max first";
    
    //! Indicates the number of outliers to be removed in each iteration
    int num{3};
    
    //! Indicates the total number of attempts, i.e., _iterations_, to remove
    //! outliers
    int n_attempts{1};
    
    //! Indicates the minimum number of observations allowed during the process
    int min_observations{15};
    
    //! A list of standard deviation multipliers for identidying outliers
    std::vector<double> multipliers = {3};
    
    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs;
    
    //! Removing if
    std::vector<std::string> whether_to_save_cond_defs;
    
  };
  
  Parameters params;
  PolicyChain stopping_condition;
  PolicyChain whether_to_save_condition;
  
  PeekingOutliersRemoval() = default;
  
  PeekingOutliersRemoval(const Parameters &p) : params{p} { };
  
  virtual void perform(Experiment *experiment) override;
  
private:
  bool removeOutliers(Experiment *experiment, const int n, const double k);
};

inline void to_json(json &j, const PeekingOutliersRemoval::Parameters &p) {
  j = json{{"name", p.name},
    {"level", p.level},
    {"order", p.order},
    {"num", p.num},
    {"n_attempts", p.n_attempts},
    {"min_observations", p.min_observations},
    {"multipliers", p.multipliers},
    {"whether_to_save_condition", p.whether_to_save_cond_defs},
    {"stopping_condition", p.stopping_cond_defs}};
}

inline void from_json(const json &j, PeekingOutliersRemoval::Parameters &p) {
  
  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);
  
  j.at("level").get_to(p.level);
  j.at("order").get_to(p.order);
  j.at("num").get_to(p.num);
  j.at("n_attempts").get_to(p.n_attempts);
  j.at("min_observations").get_to(p.min_observations);
  j.at("multipliers").get_to(p.multipliers);
  j.at("whether_to_save_condition").get_to(p.whether_to_save_cond_defs);
  
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
    
    //! Indicates which outcome variables are going to be targeted,
    //!   - control
    //!   - treatment
    //!   - both
    std::string target {"both"};
    
    //! Indicates a set of rule that is going to be used to select the target group
    //! @todo To be implemented
    PolicyChain target_policy;
    
    //! Number of trials
    int n_attempts {1};
    
    //! Number of observations to be purturbed
    int nums {5};
    
    //! Distribution of noise
    std::optional<Distribution> noise_dist = make_distribution({
      {"dist", "normal_distribution"},
      {"mean", 0},
      {"stddev", 0.1}
    });
    
    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs;
    
    
  };
  
  Parameters params;
  PolicyChain stopping_condition;
  
  FalsifyingData() = default;
  
  FalsifyingData(const Parameters &p) : params{p} { };
  
  virtual void perform(Experiment *experiment) override;
  
private:
  bool perturb(Experiment *experiment, const int n);
  bool swapGroups(Experiment *experiment, const int n);
  bool switchGroups(Experiment *experiment, const int n);
};

inline void to_json(json &j, const FalsifyingData::Parameters &p) {
  j = json{{"name", p.name},
    {"approach", p.approach},
    {"n_attempts", p.n_attempts},
    {"nums", p.nums},
    {"target", p.target},
//    {"noise", p.noise},
    {"stopping_condition", p.stopping_cond_defs}};
}

inline void from_json(const json &j, FalsifyingData::Parameters &p) {
  
  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);
  
  j.at("approach").get_to(p.approach);
  j.at("n_attempts").get_to(p.n_attempts);
  j.at("nums").get_to(p.nums);
  j.at("target").get_to(p.target);
  
  if (j.contains("noise")) {
    p.noise_dist = make_distribution(j["noise"]);
  }
  
  if (j.contains("stopping_condition"))
    j.at("stopping_condition").get_to(p.stopping_cond_defs);
}


/// Fabricating Data Hacking Strategy
///
/// @ingroup HackingStrategies
class FabricatingData final : public HackingStrategy {
  
public:
  
  /// Falsifying Data Parameters
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
    std::string target {"both"};
    
    //! Indicates a set of rule that is going to be used to select the target group
    //! @todo To be implemented
    PolicyChain target_policy;
    
    //! Number of trials
    int n_attempts {1};
    
    //! Number of observations to be purturbed
    int nums {5};
    
    //! Distribution of fabricated data
    //! @todo Check if this is even necessary or not, I think in most cases, we
    //! can probably just use the data_strategy and get over it
    std::optional<Distribution> dist = make_distribution({
      {"dist", "normal_distribution"},
      {"mean", 0},
      {"stddev", 0.1}
    });
    
    //! Stopping condition PolicyChain definitions
    std::vector<std::string> stopping_cond_defs;
    
    
  };
  
  Parameters params;
  PolicyChain stopping_condition;
  
  FabricatingData() = default;
  
  FabricatingData(const Parameters &p) : params{p} { };
  
  virtual void perform(Experiment *experiment) override;
  
private:
  bool generate(Experiment *experiment, const int n);
  bool duplicate(Experiment *experiment, const int n);
};

inline void to_json(json &j, const FabricatingData::Parameters &p) {
  j = json{{"name", p.name},
    {"approach", p.approach},
    {"n_attempts", p.n_attempts},
    {"nums", p.nums},
    {"target", p.target},
    //    {"noise", p.noise},
    {"stopping_condition", p.stopping_cond_defs}};
}

inline void from_json(const json &j, FabricatingData::Parameters &p) {
  
  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);
  
  j.at("approach").get_to(p.approach);
  j.at("n_attempts").get_to(p.n_attempts);
  j.at("nums").get_to(p.nums);
  j.at("target").get_to(p.target);
  
  if (j.contains("stopping_condition"))
    j.at("stopping_condition").get_to(p.stopping_cond_defs);
}



} // namespace sam

#endif // SAMPP_HACKINGSTRATEGIES_H
