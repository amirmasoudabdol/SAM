//===-- Experiment.h - Experiment Module ----------------------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-01-22.
//
//===----------------------------------------------------------------------===//
///
/// @file
///
///
//===----------------------------------------------------------------------===//

///
/// @defgroup   Experiment Experiment Modules
/// @brief      Experiment-related Modules
///
/// Description to come!
///

#ifndef SAMPP_EXPERIMENT_H
#define SAMPP_EXPERIMENT_H

#include "sam.h"

#include "DataStrategy.h"
#include "EffectStrategy.h"
#include "DependentVariable.h"
#include "Submission.h"
#include "TestStrategy.h"

#include <fmt/format.h>

namespace sam {

class Submission;

///
/// @brief      Experiment encapsulates data and methods needed by the Researcher to
/// conduct its research.
///
/// Experiment has access to all data-related
/// strategies, e.g., DataStrategy, TestStrategy, and EffectStrategy. Moreover,
/// it stores the actual research data in a vector of DependentVariable objects. Researcher's
/// access to actual data, and other data-related strategies always goes
/// through an Experiment object.
///
/// Moreover, the Experiment stores a copy of the ExperimentSetup where most study parameters
/// are stored in.
///
/// @ingroup    Experiment
class Experiment {

public:
  int simid{0};
  int exprid{0};
  int repid{0};

  //! Indicates if any hacking routine has been applied on the experiment
  bool is_hacked{false};
  std::vector<int> hacks_history;

  bool is_published{false};

  ExperimentSetup setup;

  std::shared_ptr<DataStrategy> data_strategy;
  std::shared_ptr<TestStrategy> test_strategy;
  std::shared_ptr<EffectStrategy> effect_strategy;

  std::vector<DependentVariable> dvs_;
  
  std::optional<std::vector<Submission>> candidates;
  void updateCandidatesList(const std::vector<Submission>& subs);
  
  void setHackedStatus(const bool status);
  void setHackedStatusOf(const std::vector<size_t> &idxs, const bool status);
  void setCandidateStatusOf(const std::vector<size_t> &idxs, const bool status);

  Experiment() = default;

  /// @brief  Constructing an Experiment object using the given JSON configuration.
  ///
  /// Starting by the initialization of an ExperimentSetup, followed by Data, Test, and
  /// Effect strategies initialization; and finally, all necessary resources will be initialized.
  Experiment(json &experiment_config);

  /// @brief Constructing an Experiment using an already initialized ExperimentSetup
  ///
  /// Since ExperimentSetup already contains the definition of Data, Test, and Effect strategies,
  /// this method accept the ExperimentSetup as it is, and only initialize those strategies.
  ///
  /// @note This can be used in cases where the underlying strategies should be preserved while
  /// some experiment parameters needs to be modified. 
  Experiment(ExperimentSetup &e);

  Experiment(ExperimentSetup &e, std::shared_ptr<DataStrategy> &ds,
             std::shared_ptr<TestStrategy> &ts,
             std::shared_ptr<EffectStrategy> &efs);

  /// These operators are returning the correct group, even if the group list is
  /// not sorted
  ///
  /// @todo I think these guys are a bit strange, they work and I'm not sure
  /// why I have them like this but I think I can do better, for now I put some
  /// guard
  /// @note I think these are bad ideas, I think they should just return the index
  /// that are being asked to, and then some other method, actually returns the group
  /// like `get_group`, and `set_group` or even a `DependentVariable operator()` why not.
  /// @note This means I need to change the DataStrategy too, and make sure that
  /// in each iteration, I start with a fresh Experiment rather than a half cleanup one.
  DependentVariable &operator[](std::size_t idx) {
    if (idx > dvs_.size())
      throw std::invalid_argument("Index out of bound.");
    
    auto g = std::find_if(dvs_.begin(), dvs_.end(), [&](auto &g) -> bool {return g.id_ == idx; });
    return *g;
  };
  
  const DependentVariable &operator[](std::size_t idx) const {
    if (idx > dvs_.size())
      throw std::invalid_argument("Index out of bound.");
    
    auto g = std::find_if(dvs_.cbegin(), dvs_.cend(), [&](auto &g) -> bool {return g.id_ == idx; });
    return *g;
  };

  auto begin() { return dvs_.begin(); };
  auto end() { return dvs_.end(); };
  
  auto begin() const { return dvs_.begin(); };
  auto end() const { return dvs_.end(); };

  /// Runs the Test Strategy
  void runTest();

  /// Set or re-set the Test Strategy
  ///
  /// @param t A reference to a Test Strategy instance
  ///
  void setTestStrategy(std::shared_ptr<TestStrategy> &ts) {
    test_strategy = ts;
  }

  /// Set or re-set the Data Strategy
  ///
  /// @param d A reference to a Data Strategy instance
  ///
  void setDataStrategy(std::shared_ptr<DataStrategy> &ds) {
    data_strategy = ds;
  }

  /// Set or re-set the Effect Strategy
  ///
  /// @param es A reference to an Effect Strategy instance.
  ///
  void setEffectSizeEstimator(std::shared_ptr<EffectStrategy> &es) {
    effect_strategy = es;
  };

  /// Helper function for the Researcher to fully initialize the experiment
  /// by generating the data, calculating the statistics and effects, as
  /// well as running the test.
  void initExperiment();

  /// Use the `data_strategy` to generate the data.

  /// @note The `data_strategy` takes over the entire experiment and
  /// populate the `measurements` based on `setup`'s parameters.
  void generateData();

  /// Run different pre-processing steps before passing the data to the
  /// Researcher.

  /// Calculate the statistics by sending the `experiment` to the
  /// `test_strategy`.
  void calculateStatistics();

  /// Iterates over the list of EffectSizeEstimators, and calculate different
  /// different estimates accordingly.
  void calculateEffects();

  void recalculateEverything();
  
  void recalculateEverythingForGroup(size_t inx);
  
  /// Clear contents of the experiment
  void clear();

private:
  /// Initialize the necessary resources
  void initResources();
};

} // namespace sam

namespace fmt {
  template <>
  struct formatter<sam::Experiment> {
    // Presentation format: 'f' - fixed, 'e' - exponential.
    char presentation = 'f';
    
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) {
      return ctx.begin();
    }
    
    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const sam::Experiment& e, FormatContext& ctx) {
      // auto format(const point &p, FormatContext &ctx) -> decltype(ctx.out()) // c++11
      // ctx.out() is an output iterator to write to.
      ctx.out() = format_to(ctx.out(), "\n\t\t");
      return format_to(ctx.out(),
                       "{}",
                       join(e.begin(), e.end(), "\n\t\t"));
    }
  
  };
}

#endif // SAMPP_EXPERIMENT_H
