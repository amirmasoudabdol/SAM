//===-- DependentVariable.cpp - DependentVariable -------------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-03-03.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This files contains the implementation of DependentVariable class.
///
//===----------------------------------------------------------------------===// 

#include "DependentVariable.h"

using namespace sam;

/// This updates all the descriptive statistics of the dependent variable
void DependentVariable::updateStats() {

  nobs_ = measurements_.size();
  mean_ = arma::mean(measurements_);
  var_ = arma::var(measurements_);
  stddev_ = arma::stddev(measurements_);
  sei_ = sqrt(var_ / nobs_);
  
}

/// This is being used by the PersistenceManager::Writer to determine the name
/// and number of columns
std::vector<std::string>
DependentVariable::Columns() {
  return {
    "gid",
    "tnobs",
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
    "is_hacked",
    "is_candidate"
  };
}

/// This offers a natural way to convert the class to a map, using the `static_cast`
///
/// @attention If you are modifying the internal of the DependentVariable, you need to
/// make sure that your changes are reflected here; otherwise, your CSV files will be
/// corrupted.
DependentVariable::operator std::map<std::string, std::string>() const {

  static std::map<std::string, std::string> record;

  record["gid"] = std::to_string(id_);

  record["tnobs"] = std::to_string(true_nobs_);
  record["nobs"] = std::to_string(nobs_);
  record["mean"] = std::to_string(mean_);
  record["var"] = std::to_string(var_);
  record["stddev"] = std::to_string(stddev_);
  record["sei"] = std::to_string(sei_);

  record["pvalue"] = std::to_string(pvalue_);
  record["effect"] = std::to_string(effect_);
  record["var_effect"] = std::to_string(var_effect_);
  record["se_effect"] = std::to_string(se_effect_);
  record["sig"] = std::to_string(sig_);
  record["side"] = std::to_string(eff_side_);

  record["is_hacked"] = std::to_string(is_hacked_);
  record["is_candidate"] = std::to_string(is_candidate_);

  return record;
}

/// Similar to the `map<string, string>()` operator, this allows for casting to
/// `arma::Row<float>`
///
/// @attention If you are modifying the internal of the DependentVariable, you need to
/// make sure that your changes are reflected here; otherwise, SAM will not export your
/// new variables to the output.
DependentVariable::operator arma::Row<float>() {
  
  return {static_cast<float>(id_),
    static_cast<float>(true_nobs_),
    static_cast<float>(nobs_),
    mean_,
    var_,
    stddev_,
    sei_,
    pvalue_,
    effect_,
    var_effect_,
    se_effect_,
    static_cast<float>(sig_),
    static_cast<float>(eff_side_),
    static_cast<float>(is_hacked_),
    static_cast<float>(is_candidate_)
  };
  
}

/// This clears and resets the internal state of the class.
void DependentVariable::clear() {
  
  is_hacked_ = false;
  is_candidate_ = false;
  
  n_added_obs = 0;
  n_removed_obs = 0;
  
  measurements_.clear();
}
