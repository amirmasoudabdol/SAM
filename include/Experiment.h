//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENT_H
#define SAMPP_EXPERIMENT_H

#include <functional>
#include <vector>

#include "sam.h"

#include "DataStrategy.h"
#include "EffectStrategy.h"
#include "ExperimentSetup.h"
#include "GroupData.h"
#include "Submission.h"
#include "TestStrategy.h"

namespace sam {

class Submission;
class EffectStrategy;

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
  int expr_uuid{0};

  //! Indicates if any hacking routine has been applied on the experiment
  bool is_hacked = false;
  std::vector<int> hacks_history;

  bool is_published = false;

  ExperimentSetup setup;

  /// \note: While I'm still confused about these, but I'm starting to feel
  /// that it actually makes sense for them to be a shared pointer. In fact, I
  /// don't really want to copy them around when I copy the experiment, I
  /// would like them to be shared. So, it seems the ownership is correct
  /// here.
  std::shared_ptr<DataStrategy> data_strategy;
  std::shared_ptr<TestStrategy> test_strategy;
  std::shared_ptr<EffectStrategy> effect_strategy;

  std::vector<GroupData> groups_;

  Experiment() = default;

  explicit Experiment(json &experiment_config);

  explicit Experiment(ExperimentSetup &e) : setup{e} {

    /// TODO: If I want to have the size of the experiment setup in the
    /// dsp_conf, I have to inject it to the `dsp_conf` here and then pass
    /// it to the builder. This way, from_json could use it as a reference
    /// and construct a properly sized object.
    data_strategy =
        std::shared_ptr<DataStrategy>(DataStrategy::build(setup.dsp_conf));

    test_strategy =
        std::shared_ptr<TestStrategy>(TestStrategy::build(setup.tsp_conf));

    effect_strategy =
        std::shared_ptr<EffectStrategy>(EffectStrategy::build(setup.esp_conf));

    initResources();
  };

  // TODO: I think this can change, I don't need to accept shared_ptr here
  Experiment(ExperimentSetup &e, std::shared_ptr<DataStrategy> &ds,
             std::shared_ptr<TestStrategy> &ts,
             std::shared_ptr<EffectStrategy> &efs)
      : setup{e}, data_strategy{ds}, test_strategy{ts}, effect_strategy{efs} {
    initResources();
  };

  GroupData &operator[](std::size_t idx) { return groups_[idx]; };
  const GroupData &operator[](std::size_t idx) const { return groups_[idx]; };

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
  /// TODO: I think I need to re-evaluate passing a shared_ptr by reference.
  void setTestStrategy(std::shared_ptr<TestStrategy> &ts) {
    test_strategy = ts;
  }

  /// Set or re-set the Data Strategy
  ///
  /// \param d A reference to a Data Strategy instance
  ///
  /// TODO: I think I need to re-evalute passing a shared_ptr by reference.
  void setDataStrategy(std::shared_ptr<DataStrategy> &ds) {
    data_strategy = ds;
  }

  /// Set or re-set the Effect Strategy
  ///
  /// \param es A reference to an Effect Strategy instance.
  ///
  /// TODO: I think I need to re-evalute passing a shared_ptr by reference.
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

  void randomize();

private:
  /// Initialize the necessary resources
  void initResources();
};

} // namespace sam

#endif // SAMPP_EXPERIMENT_H
