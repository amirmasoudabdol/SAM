//===-- Experiment.h - Experiment Module -----------------------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-01-22.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains deceleration of Experiment class, and everything connected to it
///
//===----------------------------------------------------------------------===//
///
/// @defgroup Experiment Experiment Modules
/// @brief    Experiment-related Modules
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
/// Experiment has a full access to all data-related strategies, e.g., DataStrategy,
/// TestStrategy, and EffectStrategy. It also contains the raw research data in
/// the form of list of DependentVariable(s). Moreover, the Experiment stores a copy of
/// the ExperimentSetup where most study parameters are stored in.
///
/// @ingroup    Experiment
///
class Experiment {
  
  //! Indicates if any hacking routine has been applied on the experiment
  bool is_hacked{false};
  
  //! List of hacking strategies applies on the group
  std::vector<int> hacks_history;
  
  //! Indicates whether the experiment has been published in any Journal
  bool is_published{false};
  
  //! Indicates whether any of the dependent variables are candidate for Submission
  bool has_candidates{false};
  
  //! Indicates whether any of there are any covariant variable exists in the experiment
  bool is_covariants_generated{false};

public:
  int simid{0};
  int exprid{0};
  int repid{0};
  
  //! Indicates the number of covariants
  int n_covariants{0};
  arma::Mat<int> covariants;

  //! An instance of the ExperimentSetup. All other strategies can access and query it
  //! for meta information about the Experiment
  ExperimentSetup setup;

  std::shared_ptr<DataStrategy> data_strategy;
  std::shared_ptr<TestStrategy> test_strategy;
  std::shared_ptr<EffectStrategy> effect_strategy;
  
  std::vector<DependentVariable> dvs_;

  //! @brief  List of all possible candidates from this experiment so far!
  //!
  //! This is a list of any dvs that has been flagged as submissions during the lifespan
  //! of this experiment
  //! 
  //! @note This is not yet in use!
  std::optional<std::vector<Submission>> candidates;
  
  /// Default constructor
  Experiment() = default;
  
  /// Constructs an Experiment object using the given JSON configuration.
  Experiment(json &experiment_config);
  
  /// Constructs an Experiment using an already initialized ExperimentSetup
  Experiment(ExperimentSetup &e);
  
  /// Directly constructs an Experiment from its components
  Experiment(ExperimentSetup &e,
             std::shared_ptr<DataStrategy> &ds,
             std::shared_ptr<TestStrategy> &ts,
             std::shared_ptr<EffectStrategy> &es);
  
  /// Adds a new condition
  /// @TODO TO BE IMPLEMENTED!
  void addNewCondition();
  
  /// Adds a new condition
  /// @TODO TO BE IMPLEMENTED!
  void addNewCondition(const std::vector<DependentVariable>& dvs);
  
  /// Adds a new dependent variable to the list
  void addNewDependentVariable(const DependentVariable& dv);
  
  /// Adds new submissions to the list of submissions
  void addNewCandidates(const std::vector<Submission>& subs);
  
  /// Sets the hacked status of the experiment
  void setHackedStatus(const bool status);
  
  /// Sets the published status of the experiment
  void setPublishedStatus(const bool status);
  
  /// Sets the hacked status of a group of dvs
  void setHackedStatusOf(const std::vector<size_t> &idxs, const bool status);
  
  /// Sets the candidate status of a group of dvs
  void setCandidateStatusOf(const std::vector<size_t> &idxs, const bool status);
  
  /// Returns true if the experiment is hacked
  [[nodiscard]] bool isHacked() const;
  
  /// Returns true if the experiment has been modified in anyway
  [[nodiscard]] bool isModified() const;
  
  /// Returns true if there is an candidate in the experiment
  [[nodiscard]] bool hasCandidates() const;
  
  /// Returns true if there is an candidate in the experiment
  [[nodiscard]] bool hasCovariants() const;
  
  /// Returns true if the experiment has been published by the Journal
  [[nodiscard]] bool isPublished() const;
  
  /// Returns the number of candidate DVs
  [[nodiscard]] size_t nCandidates() const;
  
  [[nodiscard]] int nCovariants() const;
  
  /**
   * @name STL-like operators and methods
   *
   * @attention begin() and end() operators do not return a sorted list of DVs. 
   * This is due to Policies re-arranging them as they filter them. If you want 
   * a sorted list of DVs, you should use the subscript operators, they make 
   * sure that the correct item is returns every time! 
   * 
   */
  ///@{
  DependentVariable &operator[](std::size_t idx);
  const DependentVariable &operator[](std::size_t idx) const;

  auto begin() { return dvs_.begin(); };
  auto end() { return dvs_.end(); };
  
  [[nodiscard]] auto begin() const { return dvs_.begin(); };
  [[nodiscard]] auto end() const { return dvs_.end(); };
  ///@}


  /// Clears and re-initializes the dependent variables
  void reset();

  /// Uses the DataStrategy to populate every DVs with raw the data.
  void generateData();
  
  /// Generates covariants data
  void generateCovariants();

  /// Asks each DependentVariable to update its general statistics, e.g., mean, var.
  void calculateStatistics();
  
  /// Uses the TestStrategy to run the statistical test
  void calculateTests();

  /// Uses the EffectStrategy to calculates the effect sizes.
  void calculateEffects();

  /// Runs calculateStatistics(), calculateTests(), and calculateEffects() in order.
  void recalculateEverything();
  
  /// Clears the content of the experiment
  void clear();
  
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
