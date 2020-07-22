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
  
  //! Caching variables
  arma::Row<double> yi;
  arma::Row<double> vi;
  arma::Row<double> wi;

  //! Rejected Submissions
  std::vector<Submission> rejection_list;
  int n_rejected{0};

  //! Journal's Selection Model/Strategy
  std::unique_ptr<SelectionStrategy> selection_strategy;

  //! Effect Estimator
  std::unique_ptr<MetaAnalysis> meta_analysis_strategy;
  
  std::vector<std::unique_ptr<MetaAnalysis>> meta_analysis_strategies;
  std::map<std::string, PersistenceManager::Writer> meta_writers;
  
  MetaAnalysisResults meta_analysis_submissions;

  struct Parameters {
    std::string name;
    int max_pubs;
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
  /// \param      s     The pointer to the given selection strategy
  ///
  void setSelectionStrategy(std::unique_ptr<SelectionStrategy> ss) {
    selection_strategy = std::move(ss);
  }

  ///
  /// \brief      Review the Submission by calling
  /// `SelectionStrategy::review()`.
  ///
  /// \param[in]  s     A reference to the Submission
  ///
  /// \return     A boolean indicating whether the Submission should be accpeted
  /// or
  ///             not.
  ///
  bool review(const Submission &s);

  ///
  /// \brief      Accept the Submission by adding it to the `publicationList`.
  ///
  /// \param[in]  s     A copy of the Submission
  ///
  void accept(const Submission &s);

  ///
  /// \brief      Rejecting the Submission!
  ///
  /// \param[in]  s     A reference to the Submission
  ///
  void reject(const Submission &s);

  bool isStillAccepting() const { return still_accepting; }

  ///
  /// Save enteries of publications_list to a CSV file.
  ///
  /// \param      simid   The index to be used for the given set.
  /// \param      writer  The output file.
  ///
  void saveSubmissions(int simid, std::ofstream &writer);
  
  void saveMetaAnalysis();

  ///
  /// Clear the publications_list vector.
  ///
  void clear() {
    publications_list.clear();
    rejection_list.clear();
    meta_analysis_submissions.clear();
    still_accepting = true;
  }
  
  void prepareForMetaAnalysis() {
    auto n = publications_list.size();
    
    yi.resize(n); vi.resize(n); wi.resize(n);
    
    for (int i{0}; i < n; ++i) {
      yi[i] = publications_list[i].group_.effect_;
      vi[i] = publications_list[i].group_.var_;
    }
    
    wi = 1./vi;
  }
  
  void runMetaAnalysis() {
    
    prepareForMetaAnalysis();
    
    for (auto &method : meta_analysis_strategies) {
      method->estimate(this);
    }
  };

};

} // namespace sam

#endif // SAMPP_JOURNAL_H
