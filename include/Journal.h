//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_JOURNAL_H
#define SAMPP_JOURNAL_H

#include "MetaAnalysis.h"
#include "SelectionStrategy.h"
#include "Submission.h"

#include "PersistenceManager.h"

#include <variant>

namespace sam {

class MetaAnalysis;

using MetaAnalysisResults = std::vector<std::variant<FixedEffectEstimator::ResultType,
                                                      RandomEffectEstimator::ResultType,
                                                      EggersTestEstimator::ResultType,
                                                      TestOfObsOverExptSig::ResultType,
                                                      TrimAndFill::ResultType,
                                                      RankCorrelation::ResultType>>;

class Journal {

  bool still_accepting = true;
  
public:
  double max_pubs;
  //! List of all accepted Submissions, i.e., publications
  std::vector<Submission> publications_list;
  int n_accepted{0};
  
  //! Rejected Submissions
  std::vector<Submission> rejection_list;
  int n_rejected{0};
  
  //! Number of significant submissions
  int n_sigs{0};
  double sum_sig_pvalue {0};
  double mean_sig_pvalue {0};
  double sum_sig_effect {0};
  double mean_sig_effect {0};
  
  //! Caching variables
  arma::Row<double> yi;
  arma::Row<double> vi;
  arma::Row<double> wi;
  
  bool is_saving_summaries;
  bool is_saving_meta;
  bool is_saving_pubs_per_sim_summaries;
  
  // Runner statistics of each simulation, this resets after each Journal's clean
  arma::running_stat_vec<arma::Row<double>> pubs_per_sim_stat_runner;
  std::unique_ptr<PersistenceManager::Writer> pubs_per_sim_stats_writer;
  
  arma::running_stat_vec<arma::Row<double>> pubs_stat_runner;
  std::unique_ptr<PersistenceManager::Writer> pubs_stats_writer;
  
  static std::vector<std::string> Columns();
  
  operator std::vector<std::string>() {
    return {
      std::to_string(n_accepted),
      std::to_string(n_rejected),
      std::to_string(n_sigs),
      std::to_string(mean_sig_pvalue),
      std::to_string(mean_sig_effect)
    };
  }
  
  operator arma::Row<double>() {
    return {
      static_cast<double>(n_accepted),
      static_cast<double>(n_rejected),
      static_cast<double>(n_sigs),
      mean_sig_pvalue,
      mean_sig_effect
    };
  }

  //! Journal's Selection Model/Strategy
  std::unique_ptr<SelectionStrategy> selection_strategy;

  //! Meta Analysis Estimators and their Stats Runner

  std::map<std::string, std::vector<std::string>> meta_columns;
  std::map<std::string, std::vector<std::string>> meta_stats_columns;
  
  std::vector<std::unique_ptr<MetaAnalysis>> meta_analysis_strategies;
  std::map<std::string, arma::running_stat_vec<arma::Row<double>>> meta_stat_runners;
  std::map<std::string, PersistenceManager::Writer> meta_writers;
  std::map<std::string, PersistenceManager::Writer> meta_stats_writers;
  
//  arma::running_stat_vec<arma::Row<double>> journal_stat_runner;
  
  // I don't think I really need this
//  arma::running_stat_vec<arma::Row<double>> journal_per_sim_stat_runner;
  
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

  explicit Journal(json &journal_config);

  explicit Journal(const Parameters &jp);

  // Journal(JournalParameters &jp,
  // SelectionStrategy::SelectionStrategyParameters &ssp);

  ///
  /// Point Journal's selection strategy to the given strategy
  ///
  /// @param      s     The pointer to the given selection strategy
  ///
  void setSelectionStrategy(std::unique_ptr<SelectionStrategy> ss) {
    selection_strategy = std::move(ss);
  }

  ///
  /// @brief      Review the Submission by calling
  /// `SelectionStrategy::review()`.
  ///
  /// @param[in]  s     A reference to the Submission
  ///
  /// @return     A boolean indicating whether the Submission should be accpeted
  /// or
  ///             not.
  ///
  bool review(Submission &s);

  ///
  /// @brief      Accept the Submission by adding it to the `publicationList`.
  ///
  /// @param[in]  s     A copy of the Submission
  ///
  void accept(const Submission &s);

  ///
  /// @brief      Rejecting the Submission!
  ///
  /// @param[in]  s     A reference to the Submission
  ///
  void reject(const Submission &s);

  bool isStillAccepting() const { return still_accepting; }

  ///
  /// Save enteries of publications_list to a CSV file.
  ///
  /// @param      simid   The index to be used for the given set.
  /// @param      writer  The output file.
  ///
  void saveSubmissions(int simid, std::ofstream &writer);
  
  void saveMetaAnalysis();
  void saveSummaries();
  void saveMetaStatsOf(std::string method);
  
  void savePulicationsPerSimSummaries();
  

  ///
  /// Clear the publications_list vector.
  ///
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
  
  void prepareForMetaAnalysis() {
    auto n = publications_list.size();
    
    yi.resize(n); vi.resize(n); wi.resize(n);
    
    for (int i{0}; i < n; ++i) {
      yi[i] = publications_list[i].group_.effect_;
      vi[i] = publications_list[i].group_.var_;
    }
    
    wi = 1./vi;
    
    // This makes sure that no study with zero variance passes to meta-analysis
    if (!wi.is_finite()) {
      arma::uvec nans = arma::find_nonfinite(wi);
      yi.shed_cols(nans);
      vi.shed_cols(nans);
      wi.shed_cols(nans);
      n -= nans.n_elem;
      spdlog::warn("{} study(-ies) have been removed from meta-anlaysis pool due to unavailability of variance", nans.n_elem);
    }
    
  }
  
  void runMetaAnalysis();
  void updateTheOverallRunner();

};

} // namespace sam

#endif // SAMPP_JOURNAL_H
