//
// Created by Amir Masoud Abdol on 2019-03-03.
//

#include "GroupData.h"

using namespace sam;

void GroupData::updateStats() {

  if (not is_stats_up_to_date) {
    nobs_ = measurements_.size();
    mean_ = arma::mean(measurements_);
    var_ = arma::var(measurements_);
    stddev_ = arma::stddev(measurements_);
    sei_ = sqrt(var_ / nobs_);
  }

  is_stats_up_to_date = true;
}

std::vector<std::string>
GroupData::Columns() {
  return {
    "gid",
    "nobs",
    "mean",
    "var",
    "stddev",
    "sei",
    "pvalue",
    "effect",
    "var_effect",
    "se_effect",
    "sig",
    "side",
    "is_hacked"
  };
}

GroupData::operator std::map<std::string, std::string>() {

  static std::map<std::string, std::string> record;

  record["gid"] = std::to_string(id_);

  record["nobs"] = std::to_string(nobs_);
  record["mean"] = std::to_string(mean_);
  record["var"] = std::to_string(var_);
  record["stddev"] = std::to_string(stddev_);
  record["sei"] = std::to_string(sei_);

  /// This can be replaced with an map.insert() and basically
  /// just insert TestStrategy's map() operator
  record["pvalue"] = std::to_string(pvalue_);
  record["effect"] = std::to_string(effect_);
  record["var_effect"] = std::to_string(var_effect_);
  record["se_effect"] = std::to_string(se_effect_);
  record["sig"] = std::to_string(sig_);
  record["side"] = std::to_string(eff_side_);

  record["is_hacked"] = std::to_string(is_hacked_);

  return record;
}

GroupData::operator arma::Row<double>() {
  
  return {static_cast<double>(id_),
    static_cast<double>(nobs_),
    mean_,
    var_,
    stddev_,
    sei_,
    pvalue_,
    effect_,
    var_effect_,
    se_effect_,
    static_cast<double>(sig_),
    static_cast<double>(eff_side_),
    static_cast<double>(is_hacked_)};
  
}

void GroupData::clear() {
  
  hacking_history_.clear();
  is_hacked_ = false;
  
  measurements_.clear();
  is_measurements_initd_ = false;
  
  is_stats_up_to_date = false;
}
