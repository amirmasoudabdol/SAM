//
// Created by Amir Masoud Abdol on 2019-03-03.
//

#ifndef SAMPP_GROUPDATA_H
#define SAMPP_GROUPDATA_H

#include "sam.h"
#include <fmt/format.h>

namespace sam {

class TestStrategy;
class EffectStrategy;
enum class HackingMethod;

enum class GroupType : int {
  Control,
  Treatment,

  //! Indicating that the group contains pooled data from other groups
  Pooled
};

/// @brief Declaration of Group class
///
/// The Group class is an abstract representation of a dependant variable
class Group {

  //! Measurements
  arma::Row<double> measurements_;
  bool is_measurements_initd_{false};

public: // Public for now
  int id_;
  GroupType gtype;

  /// --- Distribution/Population Parameters
  /// These can be their own type, and determined by the DataStrategies
  std::optional<double> true_nobs_{0};
  std::optional<double> true_mean_{0};
  std::optional<double> true_std_{0};
  //    Distribution dist;
  //    json dist_params;

  /// --- Descriptive statistics
  int nobs_{0};
  double mean_{0};
  double var_{0};
  double stddev_{0};
  double sei_{0};

  bool is_stats_up_to_date{false};

  /// --- Test statistics
  /// ... These can be their own type, and determined by the TestStrategies
  double stats_{0};
  double pvalue_{0};
  double effect_{0};
  double var_effect_{0};
  double se_effect_{0};
  int eff_side_{0};
  bool sig_{false};
  bool is_test_updated_{false};

  static std::vector<std::string> Columns();

  /// --- Hacking Meta
  bool is_hacked_{false};
  std::vector<HackingMethod> hacking_history_;
  int n_added_obs{0};
  int n_removed_obs{0};

  /// --- Meta Data
  bool is_candidate_{false};

  Group() = default;

  Group(int id_, GroupType type_) : id_{id_}, gtype{type_} {};

  Group(arma::Row<double> &data) : measurements_{data} { updateStats(); };

  /// Getter / Setter

  arma::Row<double> &measurements() { return measurements_; };
  const arma::Row<double> &measurements() const { return measurements_; };

  void set_measurements(const arma::Row<double> meas) {
    measurements_ = meas;
    nobs_ = meas.size();
    is_stats_up_to_date = false;

    true_nobs_ = nobs_;

    is_measurements_initd_ = true;
  }

  void add_measurements(const arma::Row<double> new_meas) {
    measurements_.insert_cols(nobs_, new_meas);
    n_added_obs += new_meas.n_elem;
    is_stats_up_to_date = false;
  }

  void del_measurements(const arma::uvec &idxs) {
    measurements_.shed_cols(idxs);
    n_removed_obs += idxs.n_elem;
    nobs_ -= idxs.n_elem;
    is_stats_up_to_date = false;
  }

  template <typename OStream>
  friend OStream &operator<<(OStream &os, const Group &data) {
    os << "id: " << data.id_ << " nobs: " << data.nobs_
       << " mean: " << data.mean_ << " var: " << data.var_
       << " stddev: " << data.stddev_ << " sei: " << data.sei_
       << " stats: " << data.stats_ << " pvalue: " << data.pvalue_
       << " effect: " << data.effect_ << " sig: " << data.sig_
       << " side: " << data.eff_side_;

    return os;
  }

  auto begin() { return measurements_.begin(); };
  auto end() { return measurements_.end(); };

  explicit operator std::map<std::string, std::string>() const;
  explicit operator arma::Row<double>();

  void updateStats();

  void testAgainst(const Group &other_group, TestStrategy &test_strategy);

  void effectComparedTo(const Group &other_group,
                        EffectStrategy &effect_strategy);

  void clear();
};

} // namespace sam

template <> struct fmt::formatter<sam::Group> {
  // Presentation format: 'l' - log, 'c' - csv
  char presentation = 'l';

  // Parses format specifications of the form ['l' | 'c'].
  constexpr auto parse(format_parse_context &ctx) {

    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 'f' || *it == 'e'))
      presentation = *it++;

    // Check if reached the end of the range:
    if (it != end && *it != '}')
      throw format_error("invalid format");

    // Return an iterator past the end of the parsed range:
    return it;
  }

  // Formats the point p using the parsed format specification (presentation)
  // stored in this formatter.
  template <typename FormatContext>
  auto format(const sam::Group &g, FormatContext &ctx) {
    // ctx.out() is an output iterator to write to.
    return format_to(
                     ctx.out(),
                     presentation != 'l' ? "id: {} nobs: {} mean: {:.5f} var: {:.5f} stddev: {:.5f} sei: {:.5f} stats: {:.5f} pvalue: {:.5f} effect: {:.5f} var_effect: {:.5f} se_effect: {:.5f} sig: {} side: {}" : "{},{},{:.5f},{:.5f},{:.5f},{:.5f},{:.5f},{:.5f},{:.5f},{:.5f},{:.5f},{},{}",
                     g.id_, g.nobs_, g.mean_, g.var_, g.stddev_, g.sei_, g.stats_, g.pvalue_, g.effect_, g.var_effect_, g.se_effect_, g.sig_, g.eff_side_);
    
    /// @todo this is an example of how fmt can be used to generate CSV rows, but I've not yet
    /// implemented it, I need to re-write the Writer class for this first
  }
};

#endif // SAMPP_GROUPDATA_H
