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

//! @brief      A list of meta-analysis results.
//!
//! The `std::variant` allows me to don't worry about the method of choice, and 
//! iterate over any arbitrary methods using a `std::visit`.
//!
using MetaAnalysisResults = std::vector<std::variant<
    FixedEffectEstimator::ResultType, RandomEffectEstimator::ResultType,
    EggersTestEstimator::ResultType, TestOfObsOverExptSig::ResultType,
    TrimAndFill::ResultType, RankCorrelation::ResultType>>;

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
  //! Indicates whether the Journal is still accepting new manuscripts or not!
  bool still_accepting {true};

  //! Number of accepted submissions, ie., outcomes.
  int n_accepted{0};

  //! Number of rejected submissions, i.e., outcomes
  int n_rejected{0};

  double sum_sig_pvalue{0};
  double mean_sig_pvalue{0};
  double sum_sig_effect{0};
  double mean_sig_effect{0};

  /** @name IO Flags
   */
  ///@{
  //! Indicates whether the Journal is saving the aggregated results
  bool is_saving_summaries;

  //! Indicates whether the Journal is saving the aggregated meta-analyses
  bool is_saving_meta;
  bool is_saving_pubs_per_sim_summaries;
  ///@}

  /** @name Meta-analyses Running Statistics Engines.
   */
  ///@{
  std::map<std::string, std::vector<std::string>> meta_columns;
  std::map<std::string, std::vector<std::string>> meta_stats_columns;

  std::vector<std::unique_ptr<MetaAnalysis>> meta_analysis_strategies;
  std::map<std::string, arma::running_stat_vec<arma::Row<double>>>
      meta_stat_runners;
  std::map<std::string, PersistenceManager::Writer> meta_writers;
  std::map<std::string, PersistenceManager::Writer> meta_stats_writers;
  ///@}

  /** @name Publication Running Statistics Engines.
   */
  ///@{

  // Runner statistics of each simulation, this resets after each Journal's
  // clean
  arma::running_stat_vec<arma::Row<double>> pubs_per_sim_stat_runner;
  std::unique_ptr<PersistenceManager::Writer> pubs_per_sim_stats_writer;

  arma::running_stat_vec<arma::Row<double>> pubs_stat_runner;
  std::unique_ptr<PersistenceManager::Writer> pubs_stats_writer;
  ///@}

  //! Indicates the maximum number of manuscripts to be accepted by the Journal.
  double max_pubs{};

  //! Number of accepted studies.
  //!
  //! @note       Studies are different from submissions. A study is an entire
  //!             Experiment that has been submitted and accepted by the
  //!             Journal.
  int n_studies{0};

  //! Number of significant submissions
  int n_sigs{0};

 public:

  //! List of all accepted submissions, i.e., outcomes
  std::vector<Submission> publications_list;


  //! Rejected Submissions
  std::vector<Submission> rejection_list;

  //! Caching variables
  //! The effect sizes of the accepted submissions.
  arma::Row<double> yi;

  //! The variance of the accepted submissions.
  arma::Row<double> vi;

  //! The weight of the accepted submissions, computed as 1./vi;
  arma::Row<double> wi;


  static std::vector<std::string> Columns();

  explicit operator std::vector<std::string>() const {
    return {std::to_string(n_accepted), std::to_string(n_rejected),
            std::to_string(n_sigs), std::to_string(mean_sig_pvalue),
            std::to_string(mean_sig_effect)};
  }

  explicit operator arma::Row<double>() {
    return {static_cast<double>(n_accepted), static_cast<double>(n_rejected),
            static_cast<double>(n_sigs), mean_sig_pvalue, mean_sig_effect};
  }

  //! Journal's Selection Model/Strategy
  std::unique_ptr<ReviewStrategy> review_strategy;



  //  arma::running_stat_vec<arma::Row<double>> journal_stat_runner;

  // Instrument of the stats writer...
  std::vector<std::string> submission_columns;
  std::vector<std::string> stats_columns;
  MetaAnalysisResults meta_analysis_submissions;

  struct Parameters {
    std::string name;
    int max_pubs;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Journal::Parameters, name, max_pubs);
  };

  Parameters params;

  Journal() = default;

  ~Journal() = default;

  /// Constructs a Journal from a JSON object
  explicit Journal(json &journal_config);

  ///
  /// @brief      Review the Submission by calling
  ///             `SelectionStrategy::review()`.
  ///
  /// @param[in]  s     A reference to the Submission
  ///
  /// @return     A boolean indicating whether the Submission should be accepted
  ///             or not.
  ///
  bool review(std::vector<Submission> &s);

  ///
  /// @brief      Accept the Submission by adding it to the `publicationList`.
  ///
  /// @param[in]  s     A copy of the Submission
  ///
  void accept(const std::vector<Submission> &s);

  ///
  /// @brief      Rejecting the Submission!
  ///
  /// @param[in]  s     A reference to the Submission
  ///
  void reject(const std::vector<Submission> &s);

  [[nodiscard]] bool isStillAccepting() const { return still_accepting; }

  ///
  /// Save entries of publications_list to a CSV file.
  ///
  /// @param      simid   The index to be used for the given set.
  /// @param      writer  The output file.
  ///
  void saveSubmissions(int simid, std::ofstream &writer);

  void saveMetaAnalysis();
  void saveSummaries();
  void saveMetaStatsOf(std::string method);

  void savePublicationsPerSimSummaries();

  /// Clears the publications_list vector.
  void clear() {
    publications_list.clear();
    rejection_list.clear();
    meta_analysis_submissions.clear();
    still_accepting = true;

    n_accepted = 0;
    n_rejected = 0;
    n_sigs = 0;
    sum_sig_pvalue = 0;
    sum_sig_pvalue = 0;

    pubs_per_sim_stat_runner.reset();
    //    journal_per_sim_stat_runner.reset();
  }

  /// Prepares the Journal for running meta-analysis
  void prepareForMetaAnalysis() {
    auto n = publications_list.size();

    yi.resize(n);
    vi.resize(n);
    wi.resize(n);

    for (int i{0}; i < n; ++i) {
      yi[i] = publications_list[i].dv_.effect_;
      vi[i] = publications_list[i].dv_.var_;
    }

    wi = 1. / vi;

    // This makes sure that no study with zero variance passes to meta-analysis
    if (!wi.is_finite()) {
      arma::uvec nans = arma::find_nonfinite(wi);
      yi.shed_cols(nans);
      vi.shed_cols(nans);
      wi.shed_cols(nans);
      spdlog::warn(
          "{} study(-ies) have been removed from meta-analysis pool due to "
          "unavailability of variance",
          nans.n_elem);
    }
  }

  void runMetaAnalysis();
  void updateTheOverallRunner();
};

}  // namespace sam

#endif  // SAMPP_JOURNAL_H
