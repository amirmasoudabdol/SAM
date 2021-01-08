//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_SUBMISSION_H
#define SAMPP_SUBMISSION_H

#include <map>

#include "Experiment.h"

#include <fmt/format.h>

namespace sam {

class Group;

class Submission {

  //! Somewhat of a buffer for storing and returning a csv rows
  std::map<std::string, std::string> record;

public:
  static std::vector<std::string> Columns();

  //! Simulation ID
  int simid {0};
  //! Experiment ID
  int exprid {0};
  //! Replication ID
  int repid {0};
  //! Publication ID
  int pubid {0};
  
  
  //! Index of the selected group

  //! True number of observations
  int tnobs;

  Group group_;

  Submission() = default;
  Submission(Experiment &e, const int &index);

  ~Submission() = default;

  ///
  /// @return     `true` if the Submission is significant, `false` otherwise
  ///
  [[nodiscard]] bool isSig() const { return group_.sig_; }

  template <typename OStream>
  friend OStream &operator<<(OStream &os, const Submission &s) {
    os << s.simid << ",\t" << s.repid << ",\t"  << s.pubid << ",\t" << s.tnobs << ",\t" << s.group_;

    return os;
  }

  explicit operator std::map<std::string, std::string>();
  explicit operator arma::Row<double>();
};

} // namespace sam

template <>
struct fmt::formatter<sam::Submission> {
  // Presentation format: 'f' - fixed, 'e' - exponential.
  char presentation = 'f';

  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  // Formats the point p using the parsed format specification (presentation)
  // stored in this formatter.
  template <typename FormatContext>
  auto format(const sam::Submission& s, FormatContext& ctx) {
    // auto format(const point &p, FormatContext &ctx) -> decltype(ctx.out()) // c++11
    // ctx.out() is an output iterator to write to.
    return format_to(
        ctx.out(),
        "{}, {}, {}, {}, {}, {}",
        s.simid, s.exprid, s.repid, s.pubid, s.tnobs, s.group_);
  }
};

template <>
struct fmt::formatter<std::vector<sam::Submission>> {
  // Presentation format: 'f' - fixed, 'e' - exponential.
  char presentation = 'f';
  
  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }
  
  // Formats the point p using the parsed format specification (presentation)
  // stored in this formatter.
  template <typename FormatContext>
  auto format(const std::vector<sam::Submission>& subs, FormatContext& ctx) {
    // auto format(const point &p, FormatContext &ctx) -> decltype(ctx.out()) // c++11
    // ctx.out() is an output iterator to write to.
    ctx.out() = format_to(ctx.out(), "\n\t\t");
    return format_to(ctx.out(),
                     "{}",
                     join(subs.begin(), subs.end(), "\n\t\t"));
  }
};

#endif // SAMPP_SUBMISSION_H
