//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include "Journal.h"

using namespace sam;

Journal::Journal(json &journal_config) {

  max_pubs = journal_config["max_pubs"];

  // Setting up the SelectionStrategy
  this->selection_strategy =
      SelectionStrategy::build(journal_config["selection_strategy"]);

  this->meta_analysis_strategy = MetaAnalysis::build("FixedEffectEstimator");
}

// Journal::Journal(JournalParameters &jp,
//                 SelectionStrategy::SelectionStrategyParameters &ssp) {
//    params = jp;
//    max_pubs = jp.max_pubs;
//    this->selection_strategy = SelectionStrategy::build(ssp);
//}

bool Journal::review(const Submission &s) {

  bool decision = this->selection_strategy->review(s);

  if (decision) {
    accept(s);
  } else {
    reject(s);
  }
  return decision;
}

void Journal::accept(const Submission &s) {

  publications_list.push_back(s);
  n_accepted++;

  if (publications_list.size() == max_pubs) {
    still_accepting = false;
  }
}

void Journal::reject(const Submission &s) {

  rejection_list.push_back(s);
  n_rejected++;
}

void Journal::testMeta() {
  FixedEffectEstimator fes;

  cerr << fes.estimate(publications_list);
}
