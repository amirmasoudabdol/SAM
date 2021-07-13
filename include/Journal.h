//===-- Journal.h - Journal's Deceleration --------------------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-01-24.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the deceleration of the Journal, which is responsible for
/// reviewing, accepting, and rejecting new submissions; as well as computing
/// meta-analysis metrics.
///
//===----------------------------------------------------------------------===//

#ifndef SAMPP_JOURNAL_H
#define SAMPP_JOURNAL_H

#include <variant>

#include "MetaAnalysis.h"
#include "PersistenceManager.h"
#include "ReviewStrategy.h"
#include "Submission.h"

namespace sam {

class MetaAnalysis;

//! A std::variant containing all different meta-analysis result type
using MetaAnalysisOutcome = std::variant<
FixedEffectEstimator::ResultType, RandomEffectEstimator::ResultType,
EggersTestEstimator::ResultType, TestOfObsOverExptSig::ResultType,
TrimAndFill::ResultType, RankCorrelation::ResultType>;

//! @brief      A list of meta-analysis results.
//!
//! The `std::variant` allows me to don't worry about the method of choice, and 
//! iterate over any arbitrary methods using a `std::visit`.
//!
using MetaAnalysisResults = std::vector<MetaAnalysisOutcome>;

///
/// @brief      Journal Class
///
/// The Journal class mimics the behavior of a Journal. It deals with 
/// new publications as they are being submitted for review. It is equipped with
/// a review strategy which is used to make a decision over the destiny of the 
/// newly submitted manuscript, ie., being accepted or rejected.
///
/// Additionally, it handles the meta-analysis calculation and reporting of 
/// those at the end of the simulation.
///
class Journal {
  
  //! Number of accepted submissions, ie., outcomes.
  int n_accepted{0};

  //! Number of rejected submissions, i.e., outcomes.
  int n_rejected{0};
  bool is_saving_rejected{false};

  //! Number of significant submissions
  int n_sigs{0};
  
  //! Number of accepted studies.
  //!
  //! @note       Studies are different from submissions. A study is an entire
  //!             Experiment that has been submitted and accepted by the
  //!             Journal.
  int n_studies{0};
  
  //! Indicates the maximum number of submissions to be accepted by the Journal.
  float max_pubs{};

  /** @name Publication Runner Statistic
   */
  ///@{
  //! Indicates whether the Journal is collecting and saving the aggregated
  //! statistics over _Publications.
  bool is_saving_summaries{false};
  
  //! Submissions CSV header
  std::vector<std::string> pubs_columns;
  
  //! Runner
  std::vector<std::string> pubs_stats_columns;
  
  //! Runner statistics for _Publications_.
  arma::running_stat_vec<arma::Row<float>> pubs_stats_runner;
  
  //! CSV writer for _Publications_.
  std::unique_ptr<PersistenceManager::Writer> pubs_stats_writer;
  //! @}

  
  //! List of registered meta-analysis strategies
  std::vector<std::unique_ptr<MetaAnalysis>> meta_analysis_strategies;
  
  
  /** @name Meta-analysis Runner Statistic
   */
  ///@{
  //! Indicates whether the Journal is saving the aggregated meta-analyses
  bool is_saving_meta{false};
  
  //! A vector of variants containing the outcome of each meta-analysis
  MetaAnalysisResults meta_analysis_submissions;
  
  //! A group of csv headers for each meta-analysis method
  std::map<std::string, std::vector<std::string>> meta_columns;
  
  //! A group of CSV writers each dealing with IO of one selected meta-analysis
  //! method
  std::map<std::string, PersistenceManager::Writer> meta_writers;
  
  //! AGGREGATE RUNNER ↓
  
  //! A group of stat runners aggregating information of every meta-analysis
  //! method chosen
  std::map<std::string, arma::running_stat_vec<arma::Row<float>>>
  meta_stats_runners;
  
  //! A group of csv headers for each meta-analysis aggregated method
  std::map<std::string, std::vector<std::string>> meta_stats_columns;
  
  //! A group of CSv writers each writing the *aggregated* statistics of one
  //! meta-analysis method over the entire simulation
  std::map<std::string, PersistenceManager::Writer> meta_stats_writers;
  ///@}

  /** @name Submissions Per Simulation Running Statistics Engines.
   */
  ///@{
  
  //! Indicates whether the Journal will save the aggregated statistics of
  //! submissions after each run. This basically represents the aggregate of
  //! each run.
  bool is_saving_pubs_per_sim_summaries{false};
  
  //! Runner statistics engine
  arma::running_stat_vec<arma::Row<float>> pubs_per_sim_stats_runner;
  
  //! CSV writer for pubs_per_sim_stats
  std::unique_ptr<PersistenceManager::Writer> pubs_per_sim_stats_writer;
  ///@}

 public:

  //! List of all accepted submissions, i.e., outcomes
  std::vector<Submission> publications_list;

  //! Rejected Submissions
  std::vector<Submission> rejection_list;

  //! Caching variables
  //! The effect sizes of the accepted submissions.
  arma::Row<float> yi;

  //! The variance of the accepted submissions.
  arma::Row<float> vi;

  //! The weight of the accepted submissions, computed as 1./vi;
  arma::Row<float> wi;

  //! Journal's Selection Model/Strategy
  std::unique_ptr<ReviewStrategy> review_strategy;
  
  Journal() = default;

  ~Journal() = default;

  /// Constructs a Journal from a JSON object
  explicit Journal(json &journal_config);

  /// Review the Submission
  bool review(std::vector<Submission> &s);

  /// Accepts the Submission
  void accept(const std::vector<Submission> &s);

  /// Rejects the Submission
  void reject(const std::vector<Submission> &s);

  /// Indicates whether the Journal is still accepting outcomes or not
  [[nodiscard]] bool isStillAccepting() const {
    return n_accepted < max_pubs;
  }
  
  /// Returns the number of studies
  [[nodiscard]] size_t nStudies() const {
    return n_studies;
  }
  
  /// Returns the number of publications
  [[nodiscard]] size_t nSubmissions() const {
    return n_accepted;
  }
  
  /// Returns the number of rejected publications
  [[nodiscard]] size_t nRejected() const {
    return n_rejected;
  }

  /// Saves MetaAnalysis Results
  void saveMetaAnalysis();

  /// Saves Overall Summaries
  void saveSummaries();

  /// Stores the MetaAnalysisOutcome
  void storeMetaAnalysisResult(const MetaAnalysisOutcome &res);

  /// Saves the Per Simulation summary of Publications
  void savePublicationsPerSimSummaries();

  /// Clear the Journal
  void clear();

  /// Completely resets the Journal
  void reset();

  /// Prepares the Journal for running meta-analysis
  void prepareForMetaAnalysis();

  /// Runs the meta-analysis methods
  void runMetaAnalysis();
  
  /// Updates the overall stats runners
  void updateMetaStatsRunners();
  
  //! Returns Journal's CSV header
  static std::vector<std::string> Columns();
  
  explicit operator std::vector<std::string>() const {
    return {std::to_string(n_accepted), std::to_string(n_rejected),
      std::to_string(n_sigs)};
  }
  
  explicit operator arma::Row<float>() const {
    return {static_cast<float>(n_accepted), static_cast<float>(n_rejected),
      static_cast<float>(n_sigs)};
  }
};

}  // namespace sam

#endif  // SAMPP_JOURNAL_H
