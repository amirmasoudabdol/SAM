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

#include <nlohmann/json.hpp>

#include "DecisionStrategy.h"
#include "Experiment.h"
#include "HackingStrategyTypes.h"
#include "Submission.h"
#include "Utilities.h"

namespace sam {

using json = nlohmann::json;

/// \ingroup    HackingStrategies
///
/// \brief      Abstract class for hacking strategies.
///
/// Each HackingStrategy should provide a `perform()` method. The `perform()`
/// method will take over a pointer to an Experiment and apply the implemented
/// hacking on it. Researcher decides if this is a pointer to a *fresh* copy of
/// the Experiment or a pointer to a previously "hacked" Experiment.
///
class HackingStrategy {

public:
  
  sol::state lua;
  
  /// \brief      Pure deconstuctor of the Base calss. This is important
  /// for proper deconstruction of Derived classes.
  virtual ~HackingStrategy() = 0;

  /// \brief      Factory method for building a HackingStrategy
  ///
  /// \param      config  A reference to an item of the
  ///             `json['--hacking-strategy']`. Researcher::Builder is
  ///             responsible for passing this object correctly.
  ///
  /// \return     A new HackingStrategy
  static std::unique_ptr<HackingStrategy> build(json &hacking_strategy_config);

  static std::unique_ptr<HackingStrategy> build(HackingMethod method);

  
  // These will modify the experiment
  
  void operator()(Experiment *experiment,
                          DecisionStrategy *decisionStrategy) {
//    perform(experiment, decisionStrategy);
  };

  void operator()(Experiment *experiment) {
    perform(experiment);
  };

private:
  /// \brief      Applies the hacking method on the Experiment.
  ///
  /// \param      experiment        A pointer to an Experiment.
  ///
  /// \param      decisionStrategy  A pointer to Researcher's
  ///                               DecisionStrategy. The HackingStrategy
  ///                               decides with what flag it is going to use
  ///                               the DecisionStrategy.
  virtual void perform(Experiment *experiment) = 0;
};

class NoHack final : public HackingStrategy {
public:
  NoHack(){

  };

private:
  virtual void perform(Experiment *experiment) override{};
};

///
/// \brief      Declartion of OptionalStopping hacking strategy
///
/// \ingroup    HackingStrategies
///
class OptionalStopping final : public HackingStrategy {

public:
  /// \ingroup  HackingStrategiesParameters
  ///
  /// \brief    Parameter of optional stopping method.
  ///
  struct Parameters {
    //! Placeholder for hacking strategy name
    HackingMethod name = HackingMethod::OptionalStopping;

    //! Indicates where the optional stopping should be applied!
    std::string level = "dv";

    //! Number of new observations to be added to each group
    int num = 3;

    //! Number of times that Researcher add `num` observations to each group
    int n_attempts = 1;

    //! Maximum number of times that Researcher tries to add new observations to
    //! each group
    int max_attempts = 10;
    
    std::vector<std::vector<std::string>> stopping_cond_defs;
    
  };

  Parameters params;
  PolicyChainSet stopping_pchain_set;

  OptionalStopping() = default;

  OptionalStopping(const Parameters &p)
      : params{p} {
        stopping_pchain_set = PolicyChainSet{p.stopping_cond_defs, lua};
        };

private:
  virtual void perform(Experiment *experiment) override;

private:
  ///   Add _n_ observations to all groups and return the updated experiment to
  ///   the `perform()` method.
  ///
  ///   \param experiment A pointer to the experiment
  ///   \param n number of new observations to be added
  void addObservations(Experiment *experiment, const int &n);
};

inline void to_json(json &j, const OptionalStopping::Parameters &p) {
  j = json{{"_name", p.name},
           {"level", p.level},
           {"num", p.num},
           {"n_attempts", p.n_attempts},
           {"max_attempts", p.max_attempts},
           {"stopping_condition", p.stopping_cond_defs}
  };
}

inline void from_json(const json &j, OptionalStopping::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("_name").get_to(p.name);
  j.at("level").get_to(p.level);
  j.at("num").get_to(p.num);
  j.at("n_attempts").get_to(p.n_attempts);
  j.at("max_attempts").get_to(p.max_attempts);
  
  if (j.contains("stopping_condition"))
    j.at("stopping_condition").get_to(p.stopping_cond_defs);
  
}

/// \ingroup    HackingStrategies
///
/// \brief      Declaration of Outlier Removal hacking method based on items'
///             distance from their sample mean.
///
///
class OutliersRemoval final : public HackingStrategy {
public:
  /// \ingroup HackingStrategiesParameters
  ///
  /// Parameters of Outliers Removal Strategy
  ///
  ///  ```json
  /// {
  ///     "_name": "OutliersRemoval",
  ///     "level": "dv",
  ///     "max_attempts": 1000,
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

    //! Indicates the maximum number of attempts
    int max_attempts{10};

    //! Indicates the minimum number of observations allowed during the process
    int min_observations{15};

    //! A list of standard deviation multipliers for identidying outliers
    std::vector<double> multipliers = {3};
  };

  Parameters params;

  OutliersRemoval() = default;

  OutliersRemoval(const Parameters &p)
      : params{p} {

        };

  // Submission hackedSubmission;
  virtual void perform(Experiment *experiment) override;

private:
  int removeOutliers(Experiment *experiment, const int n, const double d);
};

inline void to_json(json &j, const OutliersRemoval::Parameters &p) {
  j = json{{"_name", p.name},
           {"level", p.level},
           {"order", p.order},
           {"num", p.num},
           {"n_attempts", p.n_attempts},
           {"max_attempts", p.max_attempts},
           {"min_observations", p.min_observations},
           {"multipliers", p.multipliers}};
}

inline void from_json(const json &j, OutliersRemoval::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("_name").get_to(p.name);

  j.at("level").get_to(p.level);
  j.at("order").get_to(p.order);
  j.at("num").get_to(p.num);
  j.at("n_attempts").get_to(p.n_attempts);
  j.at("max_attempts").get_to(p.max_attempts);
  j.at("min_observations").get_to(p.min_observations);
  j.at("multipliers").get_to(p.multipliers);
}

///
/// \brief      The subjective outlier removal refers to a type of outliers
/// removal
///             where the researcher continiously lowers the threshold of
///             identifying an outlier, `k`, until it finds a significant (or
///             satisfactory) result.
///
/// \see        DecisionStrategy
/// \see        DecisionPreference
///
class SubjectiveOutlierRemoval final : public HackingStrategy {
public:
  /// \brief SubjectiveOutlierRemoval's parameters.
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
  ///    "_name": "SubjectiveOutlierRemoval",
  ///    "range": [2, 4],
  ///    "step_size": 0.1,
  ///    "min_observations": 5
  /// }
  /// ```
  /// \ingroup HackingStrategiesParameters
  struct Parameters {
    //! A placeholder for the name
    HackingMethod name = HackingMethod::SubjectiveOutlierRemoval;

    //! A vector of `{min, max}`, defining the range of `K`.
    std::vector<int> range{2, 4};

    //! Indicates the step size of walking through K's
    double step_size{0.1};

    //! Indicates minimum number of observatons
    int min_observations{5};
  };

  Parameters params;

  SubjectiveOutlierRemoval() = default;

  SubjectiveOutlierRemoval(const Parameters &p)
      : params(p){

        };

private:
  virtual void perform(Experiment *experiment) override;
};

inline void to_json(json &j, const SubjectiveOutlierRemoval::Parameters &p) {
  j = json{{"_name", p.name},
           {"range", p.range},
           {"step_size", p.step_size},
           {"min_observations", p.min_observations}};
}

inline void from_json(const json &j, SubjectiveOutlierRemoval::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("_name").get_to(p.name);
  j.at("range").get_to(p.range);
  j.at("step_size").get_to(p.step_size);
  j.at("min_observations").get_to(p.min_observations);
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

  // Submission hackedSubmission;
  virtual void perform(Experiment *experiment) override;

private:
  void pool(Experiment *experiment, int r);
};

inline void to_json(json &j, const GroupPooling::Parameters &p) {
  j = json{{"_name", p.name}, {"nums", p.nums}};
}

inline void from_json(const json &j, GroupPooling::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("_name").get_to(p.name);

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
  j = json{{"_name", p.name}};
}

inline void from_json(const json &j, ConditionDropping::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("_name").get_to(p.name);
}

} // namespace sam

#endif // SAMPP_HACKINGSTRATEGIES_H
