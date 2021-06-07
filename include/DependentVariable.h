//===-- DependentVariable.h - DependentVariable Declaration ---------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-03-03.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the declaration of the DependentVariable class which
/// mimics the dependent variables in an study
///
//===----------------------------------------------------------------------===//

#ifndef SAMPP_DEPENDENTVARIABLE_H
#define SAMPP_DEPENDENTVARIABLE_H

#include "sam.h"
#include <fmt/format.h>

namespace sam {

class TestStrategy;
class EffectStrategy;
enum class HackingMethod;

/// @brief Declaration of DependentVariable class
///
/// This class is an abstract representation of a dependant variable. It contains raw
/// data, test statistics, and effect size measurements as well as some meta data for
/// keeping track of true nobs, mean, and std values.
class DependentVariable {

  //! Raw Measurements
  arma::Row<float> measurements_;

public:
  //! Dependent variable's ID. This is being used by Policy to perform some
  //! searches
  int id_;

  /** @name Population Values
   *  These are determined by the DataStrategy
   */
  ///@{
  float true_nobs_{0};
  float true_mean_{0};
  float true_std_{0};
  ///@}

  /** @name Descriptive Statistics
   *  These will be calculated (and kept updated) by this class.
   */
  ///@{
  int nobs_{0};
  float mean_{0};
  float var_{0};
  float stddev_{0};
  float sei_{0};
  ///@}

  /** @name Test Statistics
   *  These will be calculated (and kept updated) by the TestStrategy.
   */
  ///@{
  float stats_{0};
  float pvalue_{0};
  bool sig_{false};
  ///@}
  
  /** @name Effect Size Measurement
   *  These will be calculated (and kept updated) by the EffectStrategy.
   */
  ///@{
  float effect_{0};
  float var_effect_{0};
  float se_effect_{0};
  int eff_side_{0};
  ///@}

  /** @name Hacking Information
   *  These will be calculated (and kept updated) by the Researcher, or individual
   *  HackingStrateg(-ies).
   */
  ///@{
  bool is_hacked_{false};
  int n_added_obs{0};
  int n_removed_obs{0};
  ///@}

  /// Submission Meta Data
  bool is_candidate_{false};

  DependentVariable() = default;

  explicit DependentVariable(const arma::Row<float> &data) : measurements_{data} {
    updateStats();
    true_nobs_ = nobs_;
  };
  
  /// Sets the hacking status
  void setHackedStatus(const bool status) {
    is_hacked_ = status;
  };
  
  /// Sets the candidacy status
  void setCandidateStatus(const bool status) {
    is_candidate_ = status;
  };
  
  
  /** @name Dependent Variable Status
   *
   *  These allows you to query the status of the dependent variable
   */
  ///@{
  [[nodiscard]] bool isModified() const {
    return is_hacked_ || (n_added_obs > 0) || (n_removed_obs > 0);
  }
  
  [[nodiscard]] bool isHacked() const {
    return is_hacked_;
  }
  
  [[nodiscard]] bool isCandidate() const {
    return is_candidate_;
  }
  ///@}

  /// Getter / Setter

  arma::Row<float> &measurements() { return measurements_; };
  const arma::Row<float> &measurements() const { return measurements_; };

  /// Sets the raw measurements values
  void setMeasurements(const arma::Row<float>& meas) {
    measurements_ = meas;
    nobs_ = meas.size();
    
    // We are basically redefining the values here
    true_nobs_ = nobs_;
  }

  /// Adds new measurements to the currently available data
  void addNewMeasurements(const arma::Row<float>& new_meas) {
    measurements_.insert_cols(nobs_, new_meas);
    n_added_obs += new_meas.n_elem;
    
    // Keeping the stats up to date
    updateStats();
  }

  /// Removes the measurements by their indices
  void removeMeasurements(const arma::uvec &idxs) {
    measurements_.shed_cols(idxs);
    n_removed_obs += idxs.n_elem;
    
    // Keeping the stats up to date
    updateStats();
  }

  /// Updates the descriptive statistics of the dependent variable
  void updateStats();

  /// Reset the internal state of the dependent variable
  void clear();
  
  /** @name STL-like default operators, and methods
   */
  ///@{
  auto begin() { return measurements_.begin(); };
  auto end() { return measurements_.end(); };
  
  float &operator[](std::size_t idx) {
    if (idx > measurements_.size()) {
      throw std::invalid_argument("Index out of bound.");
    }
    
    return measurements_(idx);
  }
  
  const float &operator[](std::size_t idx) const {
    if (idx > measurements_.size()) {
      throw std::invalid_argument("Index out of bound.");
    }
    
    return measurements_(idx);
  }
  ///@}
  
  
  /// @brief List the name of all avaliable variables
  static std::vector<std::string> Columns();
  explicit operator std::map<std::string, std::string>() const;
  explicit operator arma::Row<float>();
};

} // namespace sam

template <> struct fmt::formatter<sam::DependentVariable> {
  // Presentation format: 'u' - undefined, 'l' - log, 'c' - csv
  char presentation = 'u';

  // Parses format specifications of the form ['l' | 'c'].
  constexpr auto parse(format_parse_context &ctx) {

    auto it = ctx.begin();
    auto end = ctx.end();
    if (it != end && (*it == 'l' || *it == 'c')) {
      presentation = *it++;
    }

    // Check if reached the end of the range:
    if (it != end && *it != '}') {
      throw format_error("invalid format");
    }

    // Return an iterator past the end of the parsed range:
    return it;
  }

  // Formats the point p using the parsed format specification (presentation)
  // stored in this formatter.
  template <typename FormatContext>
  auto format(const sam::DependentVariable &dv, FormatContext &ctx) {
    // ctx.out() is an output iterator to write to.
    return format_to(
                     ctx.out(),
                     (presentation == 'l' or presentation == 'u') ? "id: {} tnobs: {} nobs: {} mean: {:.5f} var: {:.5f} stddev: {:.5f} sei: {:.5f} stats: {:.5f} pvalue: {:.5f} effect: {:.5f} var_effect: {:.5f} se_effect: {:.5f} sig: {} side: {} is_hacked: {} is_candidate: {}" : "{},{},{},{:.5f},{:.5f},{:.5f},{:.5f},{:.5f},{:.5f},{:.5f},{:.5f},{:.5f},{},{},{},{}",
                     dv.id_, dv.true_nobs_, dv.nobs_,
                     dv.mean_, dv.var_, dv.stddev_, dv.sei_,
                     dv.stats_, dv.pvalue_,
                     dv.effect_, dv.var_effect_, dv.se_effect_,
                     dv.sig_, dv.eff_side_,
                     dv.is_hacked_, dv.is_candidate_);
    
    /// @todo this is an example of how fmt can be used to generate CSV rows, but I've not yet
    /// implemented it, I need to re-write the Writer class for this first
  }
};

#endif // SAMPP_DEPENDENTVARIABLE_H
