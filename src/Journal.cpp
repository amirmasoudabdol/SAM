//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include "Journal.h"

using namespace sam;

std::vector<std::string>
Journal::Columns() {
  return {
    "n_accepted",
    "n_rejected"
  };
}


Journal::Journal(json &journal_config) {

  max_pubs = journal_config["max_pubs"];

  // Setting up the SelectionStrategy
  this->selection_strategy =
      SelectionStrategy::build(journal_config["selection_strategy"]);
  
  for (auto const &method : journal_config["meta_analysis_metrics"]) {
    meta_analysis_strategies.push_back(MetaAnalysis::build(method));
    
    auto cols = Columns();  // Journal Columns
    auto method_cols = MetaAnalysis::Columns(method["_name"]);
    cols.insert(cols.end(), method_cols.begin(), method_cols.end());
    
    meta_writers.try_emplace(method["_name"].get<std::string>(),
                             journal_config["output_path"].get<std::string>() +
                             journal_config["output_prefix"].get<std::string>() +
                             "_" + method["_name"].get<std::string>() + ".csv", cols);

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
  
  
  static std::vector<std::string> row;
  static std::vector<std::string> mrow;
  
  for (auto &res : meta_analysis_submissions) {
    
    // This uses the conversion operator to cast the Journal to vector of strings
    row = *this;
    
    std::visit(overload{
      [&](FixedEffectEstimator::ResultType &res) {
        mrow = res;
        row.insert(row.end(), mrow.begin(), mrow.end());
        meta_writers["FixedEffectEstimator"].write(row);
      },
      [&](RandomEffectEstimator::ResultType &res) {
        mrow = res;
        row.insert(row.end(), mrow.begin(), mrow.end());
        meta_writers["RandomEffectEstimator"].write(row);
      },
      [&](EggersTestEstimator::ResultType &res) {
        mrow = res;
        row.insert(row.end(), mrow.begin(), mrow.end());
        meta_writers["EggersTestEstimator"].write(row);
      },
      [&](TestOfObsOverExptSig::ResultType &res) {
        mrow = res;
        row.insert(row.end(), mrow.begin(), mrow.end());
        meta_writers["TestOfObsOverExptSig"].write(row);
      },
      [&](TrimAndFill::ResultType &res) {
        mrow = res;
        row.insert(row.end(), mrow.begin(), mrow.end());
        meta_writers["TrimAndFill"].write(row);
      },
      [&](RankCorrelation::ResultType &res) {
        mrow = res;
        row.insert(row.end(), mrow.begin(), mrow.end());
        meta_writers["RankCorrelation"].write(row);
      }
    }, res);
  }
}
