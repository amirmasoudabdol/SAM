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
  
  for (auto const &method : journal_config["meta_analysis_metrics"]) {
    meta_analysis_strategies.push_back(MetaAnalysis::build(method));
    
    meta_writers.try_emplace(method.get<std::string>(),
                             journal_config["output_path"].get<std::string>() +
                             journal_config["output_prefix"].get<std::string>() +
                             "_" + method.get<std::string>() + ".csv", MetaAnalysis::Columns(method));

  }
  
  /// Removing the higher level information because I don't want
  /// them to be written to the config file again.
  journal_config.erase("output_path");
  journal_config.erase("output_prefix");
}

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

void Journal::saveMetaAnalysis() { 
  for (auto &res : meta_analysis_submissions) {
    std::visit(overload{
      [&](FixedEffectEstimator::ResultType &res) {
        std::vector<std::string> row = res;
        meta_writers["FixedEffectEstimator"].write(row);
      },
      [&](RandomEffectEstimator::ResultType &res) {
        std::vector<std::string> row = res;
        meta_writers["RandomEffectEstimator"].write(row);
      },
      [&](EggersTestEstimator::ResultType &res) {
        std::vector<std::string> row = res;
        meta_writers["EggersTestEstimator"].write(row);
      }
    }, res);
  }
}
