//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENT_H
#define SAMPP_EXPERIMENT_H

#include "sam.h"

#include "DataStrategy.h"
#include "EffectStrategy.h"
#include "GroupData.h"
#include "Submission.h"
#include "TestStrategy.h"

#include <fmt/format.h>

namespace sam {

///
/// \brief      Experiment class declaration
///
/// \note       This could be an abstract class. The abstract will define the body
///             an experiment while subclasses customize it for different type of
///             experiments. This can be used to save space because I can only
///             define relevant variables for each type of experiment
///
class Experiment {

public:
  int exprid{0};
  
  int repid{0};

  //! Indicates if any hacking routine has been applied on the experiment
  bool is_hacked = false;
  std::vector<int> hacks_history;

  bool is_published = false;

  ExperimentSetup setup;

  std::shared_ptr<DataStrategy> data_strategy;
  std::shared_ptr<TestStrategy> test_strategy;
  std::shared_ptr<EffectStrategy> effect_strategy;

  std::vector<GroupData> groups_;

  Experiment() = default;

  Experiment(json &experiment_config);

  Experiment(ExperimentSetup &e);

  Experiment(ExperimentSetup &e, std::shared_ptr<DataStrategy> &ds,
             std::shared_ptr<TestStrategy> &ts,
             std::shared_ptr<EffectStrategy> &efs);

  /// These operators are returning the correct group, even if the group list is
  /// not sorted
  ///
  /// \todo: I think these guys are a bit strange, they work and I'm not sure
  /// why I have them like this but I think I can do better, for now I put some
  /// guard
  /// \note I think these are bad ideas, I think they should just return the index
  /// that are being asked to, and then some other method, actually returns the group
  /// like `get_group`, and `set_group` or even a `Group operator()` why not.
  /// \note This means I need to change the DataStrategy too, and make sure that
  /// in each iteration, I start with a fresh Experiment rather than a half cleanup one.
  GroupData &operator[](std::size_t idx) {
    if (idx > groups_.size())
      throw std::invalid_argument("Index out of bound.");
    
    auto g = std::find_if(groups_.begin(), groups_.end(), [&](auto &g) -> bool {return g.id_ == idx; });
    return *g;
  };
  
  const GroupData &operator[](std::size_t idx) const {
    if (idx > groups_.size())
      throw std::invalid_argument("Index out of bound.");
    
    auto g = std::find_if(groups_.cbegin(), groups_.cend(), [&](auto &g) -> bool {return g.id_ == idx; });
    return *g;
  };

  GroupData &get_group(std::size_t idx) { return groups_[idx]; };
  const GroupData &get_group(std::size_t idx) const { return groups_[idx]; };

  auto begin() { return groups_.begin(); };
  auto end() { return groups_.end(); };

  /// Runs the Test Strategy
  void runTest();

  /// Set or re-set the Test Strategy
  ///
  /// \param t A reference to a Test Strategy instance
  ///
  void setTestStrategy(std::shared_ptr<TestStrategy> &ts) {
    test_strategy = ts;
  }

  /// Set or re-set the Data Strategy
  ///
  /// \param d A reference to a Data Strategy instance
  ///
  void setDataStrategy(std::shared_ptr<DataStrategy> &ds) {
    data_strategy = ds;
  }

  /// Set or re-set the Effect Strategy
  ///
  /// \param es A reference to an Effect Strategy instance.
  ///
  void setEffectSizeEstimator(std::shared_ptr<EffectStrategy> &es) {
    effect_strategy = es;
  };

  /// Helper function for the Researcher to fully initilize the experiment
  /// by generating the data, calculating the statistics and effects, as
  /// well as running the test.
  void initExperiment();

  /// Use the `data_strategy` to generate the data.

  /// \note The `data_strategy` takes over the entire experiment and
  /// populate the `measurements` based on `setup`'s parameters.
  void generateData();

  /// Run different pre-processing steps before passing the data to the
  /// Researcher.

  /// \note `pre_processing_steps` lists the available steps and their order.
  void preProcessData();

  /// Calculate the statistics by sending the `experiment` to the
  /// `test_strategy`.
  void calculateStatistics();

  /// Iterates over the list of EffectSizeEstimators, and calculate different
  /// different estimates accordingly.
  void calculateEffects();

  void recalculateEverything();
  
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
      return format_to(
                       ctx.out(),
                       "{}, {}",
                       e.repid, e.is_hacked);
    }
  };
}

#endif // SAMPP_EXPERIMENT_H
