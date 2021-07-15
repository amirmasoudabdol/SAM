//===-- Journal.cpp - Journal's Implementation ----------------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-01-24.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the implementation of the Journal class. It contains the
/// code the reviewing process, computing meta-analysis and handling the Journal
/// related IO.
///
//===----------------------------------------------------------------------===//

#include "Journal.h"
#include <algorithm>

using namespace sam;

std::vector<std::string> Journal::Columns() {
  return {"n_accepted", "n_rejected", "n_sig", "mean_sig_pvalue",
          "mean_sig_effect"};
}

///
/// This makes sure that everything in the Journal are set up correctly:
///
/// - Instantiates the Review Strategy
/// - Prepares the output files, and their column names
/// - Prepares the stat runners based on users preference
///
/// @param      journal_config  The journal configuration
///
Journal::Journal(json &journal_config) {
  spdlog::debug("Initializing the Journal.");

  // Setting up the ReviewStrategy
  this->review_strategy =
      ReviewStrategy::build(journal_config["review_strategy"]);

  max_pubs = journal_config["max_pubs"];

  // IO Flags
  is_saving_rejected = journal_config["save_rejected"];
  is_saving_meta = journal_config["save_meta"];
  is_saving_summaries = journal_config["save_overall_summaries"];
  is_saving_pubs_per_sim_summaries =
      journal_config["save_pubs_per_sim_summaries"];

  // For each given method, we prepare their output columns names, and an
  // output file. If we are saving summaries, we initialize a stats runner, as
  // well as appropriate column names, and output file.
  for (auto const &method : journal_config["meta_analysis_metrics"]) {

    // Registering the new strategy
    auto method_name = method["name"].get<std::string>();
    meta_analysis_strategies.push_back(MetaAnalysis::build(method));

    // Collecting its column name
    auto cols = MetaAnalysis::Columns(method["name"]);

    // Registering the method's columns to the list
    meta_columns.try_emplace(method_name, cols);

    // If saving meta analysis results, we'll create a file for each 
    // method, and register the writer to the list of meta writers table
    if (is_saving_meta) {
      meta_writers.try_emplace(
          method_name,
          journal_config["output_path"].get<std::string>() +
              journal_config["output_prefix"].get<std::string>() + "_" +
              method_name + ".csv",
          cols);
    }

    // If saving the summaries, ie., the aggregated statistics of each output, 
    // we register their column names to the column database
    if (is_saving_summaries) {
      meta_stats_runners.try_emplace(
          method_name, arma::running_stat_vec<arma::Row<float>>());

      // Prepare the column names for each aggregated method
      std::vector<std::string> meta_stats_cols;
      for (auto &col : meta_columns[method_name]) {
        meta_stats_cols.push_back("mean_" + col);
        meta_stats_cols.push_back("min_" + col);
        meta_stats_cols.push_back("max_" + col);
        meta_stats_cols.push_back("var_" + col);
        //        meta_stats_cols.push_back("stddev_" + col);
      }

      meta_stats_columns[method_name] = meta_stats_cols;

      // and register a writer for each method to the meta summary writer table
      meta_stats_writers.try_emplace(
          method_name,
          journal_config["output_path"].get<std::string>() +
              journal_config["output_prefix"].get<std::string>() + "_" +
              method_name + "_Summaries.csv",
          meta_stats_columns[method_name]);
    }
  }

  // Getting submission's column names
  pubs_columns = Submission::Columns();

  // Preparing column names for the aggregated output
  for (auto &col : pubs_columns) {
    pubs_stats_columns.push_back("mean_" + col);
    pubs_stats_columns.push_back("min_" + col);
    pubs_stats_columns.push_back("max_" + col);
    pubs_stats_columns.push_back("var_" + col);
    //    stats_columns.push_back("stddev_" + col);
  }

  if (is_saving_summaries) {
    pubs_stats_writer = std::make_unique<PersistenceManager::Writer>(journal_config["output_path"].get<std::string>() +
                journal_config["output_prefix"].get<std::string>() +
                "_Publications_Summaries.csv",
            pubs_stats_columns);
  }

  if (is_saving_pubs_per_sim_summaries) {
    pubs_per_sim_stats_writer = std::make_unique<PersistenceManager::Writer>(journal_config["output_path"].get<std::string>() +
                journal_config["output_prefix"].get<std::string>() +
                "_Publications_Per_Sim_Summaries.csv",
        pubs_stats_columns);
  }

  /// Removing the higher level information because I don't want
  /// them to be written to the config file again.
  journal_config.erase("output_path");
  journal_config.erase("output_prefix");
  journal_config.erase("save_overall_summaries");
  journal_config.erase("save_rejected");
  journal_config.erase("save_meta");
  journal_config.erase("save_pubs_per_sim_summaries");
}

///
/// Sends the submission to the review strategy, and based on its verdict it
/// either accept() or reject() the submission.
///
/// @param      subs  A list of submissions
///
/// @return     Returns `true` if the submission is accepted, otherwise it
///             returns false.
///
bool Journal::review(std::vector<Submission> &subs) {
  bool decision = this->review_strategy->review(subs);

  if (decision) {
    accept(subs);

    // Stats runner over all publications of this journal
    if (is_saving_pubs_per_sim_summaries) {
      for (auto &s : subs) {
        pubs_per_sim_stats_runner(static_cast<arma::Row<float>>(s));
      }
    }

    // Stat runner over all simulations
    if (is_saving_summaries) {
      for (auto &s : subs) {
        pubs_stats_runner(static_cast<arma::Row<float>>(s));
      }
    }

  } else {
    reject(subs);
  }

  return decision;
}

///
/// Adds the accepted submission to the list of publications, and updates some
/// of the necessary internals of the Journal to be able to keep track of the
/// number of publications, studies, etc.
///
/// @param[in]  subs  A list of submissions
///
void Journal::accept(const std::vector<Submission> &subs) {
  //  publications_list.push_back(subs);
  publications_list.insert(publications_list.end(), subs.begin(), subs.end());
  n_accepted += subs.size();

  n_studies++;

  spdlog::trace("Accepted Submissions: {}", subs);

}

///
/// Adds the rejected submissions to the list of rejected submissions, and keeps
/// the internal of the Journal up-to-date.
///
/// @param[in]  subs  A list of submissions
///
void Journal::reject(const std::vector<Submission> &subs) {

  if (is_saving_rejected) {
    rejection_list.insert(rejection_list.end(), subs.begin(), subs.end());
  }

  n_rejected += subs.size();
}

/// Loops through the meta-analysis methods and run them, and update their stats
/// runners
void Journal::runMetaAnalysis() {
  if (!meta_analysis_strategies.empty()) {
    prepareForMetaAnalysis();
  }

  for (auto &method : meta_analysis_strategies) {
    method->estimate(this);

    if (is_saving_summaries) {
      updateMetaStatsRunners();
    }
  }
}

/// Adds the given meta analysis outcome to the list of collected meta-analysis.
void Journal::storeMetaAnalysisResult(const MetaAnalysisOutcome &res) {
  meta_analysis_submissions.push_back(res);
}


/// 
/// This prepares the Journal for running meta-analysis. I mainly designed this
/// to introduce some caching that I don't have to compute everything every
/// time. So, with this, Journal prepares the #vi, #yi, and #wi once and pass 
/// them to the meta analysis methods.
/// 
void Journal::prepareForMetaAnalysis() {
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
  // @todo this needs to be handled in a nicer way...
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


/// Clears the history of publications, rejections, and runner statistics...
void Journal::clear() {
  publications_list.clear();
  rejection_list.clear();
  meta_analysis_submissions.clear();
  
  n_studies = 0;
  n_accepted = 0;
  n_rejected = 0;
  n_sigs = 0;
  
  pubs_per_sim_stats_runner.reset();
}

/// In addition to clearing the Journal, it resets some of the overall stats 
/// runners that are not usually being cleared by clear()
void Journal::reset() {
  clear();

  pubs_stats_runner.reset();

  // Resets the individual runners
  std::for_each(meta_stats_runners.begin(), meta_stats_runners.end(), [](auto &item){
    item.second.reset();
  });

}


/// Updates the overall meta stats runners
void Journal::updateMetaStatsRunners() {
  auto record = meta_analysis_submissions.back();

  std::visit(overload{[&](FixedEffectEstimator::ResultType &res) {
                        meta_stats_runners["FixedEffectEstimator"](
                            static_cast<arma::Row<float>>(res));
                      },
                      [&](RandomEffectEstimator::ResultType &res) {
                        meta_stats_runners["RandomEffectEstimator"](
                            static_cast<arma::Row<float>>(res));
                      },
                      [&](EggersTestEstimator::ResultType &res) {
                        meta_stats_runners["EggersTestEstimator"](
                            static_cast<arma::Row<float>>(res));
                      },
                      [&](TestOfObsOverExptSig::ResultType &res) {
                        meta_stats_runners["TestOfObsOverExptSig"](
                            static_cast<arma::Row<float>>(res));
                      },
                      [&](TrimAndFill::ResultType &res) {
                        meta_stats_runners["TrimAndFill"](
                            static_cast<arma::Row<float>>(res));
                      },
                      [&](RankCorrelation::ResultType &res) {
                        meta_stats_runners["RankCorrelation"](
                            static_cast<arma::Row<float>>(res));
                      }},
             record);
}

/// Saves the meta analytics results
void Journal::saveMetaAnalysis() {
  static std::vector<std::string> mrow;

  for (auto &res : meta_analysis_submissions) {
    std::visit(overload{[&](FixedEffectEstimator::ResultType &res) {
                          mrow = res;
                          meta_writers["FixedEffectEstimator"].write(mrow);
                        },
                        [&](RandomEffectEstimator::ResultType &res) {
                          mrow = res;
                          meta_writers["RandomEffectEstimator"].write(mrow);
                        },
                        [&](EggersTestEstimator::ResultType &res) {
                          mrow = res;
                          meta_writers["EggersTestEstimator"].write(mrow);
                        },
                        [&](TestOfObsOverExptSig::ResultType &res) {
                          mrow = res;
                          meta_writers["TestOfObsOverExptSig"].write(mrow);
                        },
                        [&](TrimAndFill::ResultType &res) {
                          mrow = res;
                          meta_writers["TrimAndFill"].write(mrow);
                        },
                        [&](RankCorrelation::ResultType &res) {
                          mrow = res;
                          meta_writers["RankCorrelation"].write(mrow);
                        }},
               res);
  }
}

/// Saves the publications and meta stats runners
void Journal::saveSummaries() {
  spdlog::info("Saving Overall Statistics Summaries...");

  static std::map<std::string, std::string> record;
  
  // This is because of the static definition of the record
  record.clear();

  // Preparing and writing the summary of every meta-analysis method
  for (auto &item : meta_stats_runners) {
    for (int c{0}; c < meta_columns[item.first].size(); ++c) {
      record["mean_" + meta_columns[item.first][c]] =
          std::to_string(meta_stats_runners[item.first].mean()[c]);
      record["min_" + meta_columns[item.first][c]] =
          std::to_string(meta_stats_runners[item.first].min()[c]);
      record["max_" + meta_columns[item.first][c]] =
          std::to_string(meta_stats_runners[item.first].max()[c]);
      record["var_" + meta_columns[item.first][c]] =
          std::to_string(meta_stats_runners[item.first].var()[c]);

      //      if (meta_stat_runners[item.first].stddev().empty())
      //        record["stddev_" + meta_columns[item.first][c]] = "0";
      //      else
      //        record["stddev_" + meta_columns[item.first][c]] =
      //        std::to_string(meta_stat_runners[item.first].stddev()[c]);
    }

    meta_stats_writers[item.first].write(record);
  }

  // Cleaning record since I'd need a new set of key-values
  record.clear();

  // Preparing the summary of all publications
  for (int c{0}; c < pubs_columns.size(); ++c) {
    record["mean_" + pubs_columns[c]] =
        std::to_string(pubs_stats_runner.mean()[c]);
    record["min_" + pubs_columns[c]] =
        std::to_string(pubs_stats_runner.min()[c]);
    record["max_" + pubs_columns[c]] =
        std::to_string(pubs_stats_runner.max()[c]);
    record["var_" + pubs_columns[c]] =
        std::to_string(pubs_stats_runner.var()[c]);

    //    if (pubs_stat_runner.stddev().empty())
    //      record["stddev_" + pubs_columns[c]] = "0";
    //    else
    //      record["stddev_" + pubs_columns[c]] =
    //      std::to_string(pubs_stat_runner.stddev()[c]);
  }


  pubs_stats_writer->write(record);
}

///
/// @brief Saving the runner statistics of each batch of publications in Journal
///
void Journal::savePublicationsPerSimSummaries() {
  static std::map<std::string, std::string> record;

  for (int c{0}; c < pubs_columns.size(); ++c) {
    record["mean_" + pubs_columns[c]] =
        std::to_string(pubs_per_sim_stats_runner.mean()[c]);
    record["min_" + pubs_columns[c]] =
        std::to_string(pubs_per_sim_stats_runner.min()[c]);
    record["max_" + pubs_columns[c]] =
        std::to_string(pubs_per_sim_stats_runner.max()[c]);
    record["var_" + pubs_columns[c]] =
        std::to_string(pubs_per_sim_stats_runner.var()[c]);

    //    if (pubs_per_sim_stat_runner.stddev().empty())
    //      record["stddev_" + pubs_columns[c]] = "0";
    //    else
    //      record["stddev_" + pubs_columns[c]] =
    //      std::to_string(pubs_per_sim_stat_runner.stddev()[c]);
  }

  pubs_per_sim_stats_writer->write(record);

  /// Resetting the runner statistics
  pubs_per_sim_stats_runner.reset();
}
