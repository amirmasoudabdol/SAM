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
  
  is_saving_meta = journal_config["save_meta"];
  is_saving_summaries = journal_config["save_overall_summaries"];
  is_saving_pubs_per_sim_summaries = journal_config["save_pubs_per_sim_summaries"];
  
  for (auto const &method : journal_config["meta_analysis_metrics"]) {
    meta_analysis_strategies.push_back(MetaAnalysis::build(method));
    
    auto method_name = method["name"].get<std::string>();
    //      auto cols = Columns();  // Journal Columns
    auto cols = MetaAnalysis::Columns(method["name"]);
    //      cols.insert(cols.end(), method_cols.begin(), method_cols.end());
    
    meta_columns.try_emplace(method_name, cols);
    
    if (is_saving_meta) {
      meta_writers.try_emplace(method_name,
                               journal_config["output_path"].get<std::string>() +
                               journal_config["output_prefix"].get<std::string>() +
                               "_" + method_name + ".csv", cols);
    }
    
    if (is_saving_summaries) {
      meta_stat_runners.try_emplace(method_name, arma::running_stat_vec<arma::Row<double>>());
      
      std::vector<std::string> meta_stats_cols;
      for (auto &col : meta_columns[method_name]) {
        meta_stats_cols.push_back("mean_" + col);
        meta_stats_cols.push_back("min_" + col);
        meta_stats_cols.push_back("max_" + col);
        meta_stats_cols.push_back("var_" + col);
        meta_stats_cols.push_back("stddev_" + col);
      }
      
      meta_stats_columns[method_name] = meta_stats_cols;
      
      meta_stats_writers.try_emplace(method_name,
                                     journal_config["output_path"].get<std::string>() +
                                     journal_config["output_prefix"].get<std::string>() +
                                     "_" + method_name + "_Summaries.csv", meta_stats_columns[method_name]);
    }
    
  }

  submission_columns = Submission::Columns();
  
  for (auto &col : submission_columns) {
    stats_columns.push_back("mean_" + col);
    stats_columns.push_back("min_" + col);
    stats_columns.push_back("max_" + col);
    stats_columns.push_back("var_" + col);
    stats_columns.push_back("stddev_" + col);
  }
  
  if (is_saving_pubs_per_sim_summaries) {
    pubs_per_sim_stats_writer = std::make_unique<PersistenceManager::Writer>(journal_config["output_path"].get<std::string>() +
                                                                   journal_config["output_prefix"].get<std::string>() +
                                                                   "_Publications_Per_Sim_Summaries.csv", stats_columns);
  }
  
  if (is_saving_summaries){
    pubs_stats_writer = std::make_unique<PersistenceManager::Writer>(journal_config["output_path"].get<std::string>() +
                                                                     journal_config["output_prefix"].get<std::string>() +
                                                                     "_Publications_Summaries.csv", stats_columns);
  }
    
  
  /// Removing the higher level information because I don't want
  /// them to be written to the config file again.
  journal_config.erase("output_path");
  journal_config.erase("output_prefix");
  journal_config.erase("save_overall_summaries");
}

bool Journal::review(Submission &s) {

  bool decision = this->selection_strategy->review(s);

  if (decision) {
    accept(s);
    
    if (is_saving_summaries) {
        pubs_stat_runner(static_cast<arma::rowvec>(s));
    }
    
  } else {
    reject(s);
  }
  return decision;
}

void Journal::accept(const Submission &s) {

  publications_list.push_back(s);
  n_accepted++;

  /// \todo Maybe I should calculate the publications stats here
  
  if (publications_list.size() == max_pubs) {
    still_accepting = false;
  }
}

void Journal::reject(const Submission &s) {

  rejection_list.push_back(s);
  n_rejected++;
}



void Journal::runMetaAnalysis() {
  
  prepareForMetaAnalysis();
  
  for (auto &method : meta_analysis_strategies) {
    method->estimate(this);
    
    if (is_saving_summaries)
      updateTheOverallRunner();
  }
}


void Journal::updateTheOverallRunner() {
  
  auto record = meta_analysis_submissions.back();
  
  std::visit(overload{
    [&](FixedEffectEstimator::ResultType &res) {
        meta_stat_runners["FixedEffectEstimator"](static_cast<arma::rowvec>(res));
    },
    [&](RandomEffectEstimator::ResultType &res) {
        meta_stat_runners["RandomEffectEstimator"](static_cast<arma::rowvec>(res));
    },
    [&](EggersTestEstimator::ResultType &res) {
        meta_stat_runners["EggersTestEstimator"](static_cast<arma::rowvec>(res));
    },
    [&](TestOfObsOverExptSig::ResultType &res) {
        meta_stat_runners["TestOfObsOverExptSig"](static_cast<arma::rowvec>(res));
    },
    [&](TrimAndFill::ResultType &res) {
        meta_stat_runners["TrimAndFill"](static_cast<arma::rowvec>(res));
    },
    [&](RankCorrelation::ResultType &res) {
        meta_stat_runners["RankCorrelation"](static_cast<arma::rowvec>(res));
    }
  }, record);
}

void Journal::saveMetaAnalysis() {
  
  
//  static std::vector<std::string> row;
  static std::vector<std::string> mrow;
  
  for (auto &res : meta_analysis_submissions) {
    
    // This uses the conversion operator to cast the Journal to vector of strings
//    row = *this;
    
    std::visit(overload{
      [&](FixedEffectEstimator::ResultType &res) {
        
        mrow = res;
//        row.insert(row.end(), mrow.begin(), mrow.end());
        meta_writers["FixedEffectEstimator"].write(mrow);
      },
      [&](RandomEffectEstimator::ResultType &res) {
        mrow = res;
//        row.insert(row.end(), mrow.begin(), mrow.end());
        meta_writers["RandomEffectEstimator"].write(mrow);
      },
      [&](EggersTestEstimator::ResultType &res) {
        mrow = res;
//        row.insert(row.end(), mrow.begin(), mrow.end());
        meta_writers["EggersTestEstimator"].write(mrow);
      },
      [&](TestOfObsOverExptSig::ResultType &res) {
        mrow = res;
//        row.insert(row.end(), mrow.begin(), mrow.end());
        meta_writers["TestOfObsOverExptSig"].write(mrow);
      },
      [&](TrimAndFill::ResultType &res) {
        mrow = res;
//        row.insert(row.end(), mrow.begin(), mrow.end());
        meta_writers["TrimAndFill"].write(mrow);
      },
      [&](RankCorrelation::ResultType &res) {
        mrow = res;
//        row.insert(row.end(), mrow.begin(), mrow.end());
        meta_writers["RankCorrelation"].write(mrow);
      }
    }, res);
  }
}

void Journal::saveSummaries() {
  
  for (auto &item : meta_stat_runners) {
    
    std::map<std::string, std::string> record;
    
    for (int c{0}; c < meta_columns[item.first].size(); ++c) {
      record["mean_" + meta_columns[item.first][c]] = std::to_string(meta_stat_runners[item.first].mean()[c]);
      record["min_" + meta_columns[item.first][c]] = std::to_string(meta_stat_runners[item.first].min()[c]);
      record["max_" + meta_columns[item.first][c]] = std::to_string(meta_stat_runners[item.first].max()[c]);
      record["var_" + meta_columns[item.first][c]] = std::to_string(meta_stat_runners[item.first].var()[c]);
      
      if (meta_stat_runners.size() > 2)
        record["stddev_" + meta_columns[item.first][c]] = std::to_string(meta_stat_runners[item.first].stddev()[c]);
      else
        record["stddev_" + meta_columns[item.first][c]] = "0";
    }
    
    meta_stats_writers[item.first].write(record);
    
  }
  
  std::map<std::string, std::string> record;
  
  for (int c{0}; c < submission_columns.size(); ++c) {
    record["mean_" + submission_columns[c]] = std::to_string(pubs_stat_runner.mean()[c]);
    record["min_" + submission_columns[c]] = std::to_string(pubs_stat_runner.min()[c]);
    record["max_" + submission_columns[c]] = std::to_string(pubs_stat_runner.max()[c]);
    record["var_" + submission_columns[c]] = std::to_string(pubs_stat_runner.var()[c]);
    record["stddev_" + submission_columns[c]] = std::to_string(pubs_stat_runner.stddev()[c]);
  }
  
  pubs_stats_writer->write(record);
  
}


void Journal::savePulicationsPerSimSummaries() {
  
  static std::map<std::string, std::string> record;
  
  /// Calculating the statistics
  pubs_per_sim_stat_runner.reset();
  for (auto &s : publications_list) {
    arma::rowvec row = s;
    pubs_per_sim_stat_runner(row);
  }
  
  for (int c{0}; c < submission_columns.size(); ++c) {
    record["mean_" + submission_columns[c]] = std::to_string(pubs_per_sim_stat_runner.mean()[c]);
    record["min_" + submission_columns[c]] = std::to_string(pubs_per_sim_stat_runner.min()[c]);
    record["max_" + submission_columns[c]] = std::to_string(pubs_per_sim_stat_runner.max()[c]);
    record["var_" + submission_columns[c]] = std::to_string(pubs_per_sim_stat_runner.var()[c]);
    record["stddev_" + submission_columns[c]] = std::to_string(pubs_per_sim_stat_runner.stddev()[c]);
  }
  
  pubs_per_sim_stats_writer->write(record);
  
}
