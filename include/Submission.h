//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_SUBMISSION_H
#define SAMPP_SUBMISSION_H

#include <map>

#include "Experiment.h"

#include <fmt/format.h>

namespace sam {

class GroupData;

class Submission {

  std::vector<std::string> columns;
  std::map<std::string, std::string> record;

public:
  std::vector<std::string> Columns();

  //! Simulation ID
  int simid {0};
  //! Experiment ID
  int exprid {0};
  //! Replicaiton ID
  int repid {0};
  //! Publication ID
  int pubid {0};
  
  
  //! Index of the selected group

  //! True number of observations
  int tnobs;

  GroupData group_;

  Submission() = default;
  Submission(Experiment &e, const int &index);

  ~Submission() = default;

  ///
  /// \return     `true` if the Submission is significant, `false` otherwise
  ///
  bool isSig() const { return group_.sig_; }

  template <typename OStream>
  friend OStream &operator<<(OStream &os, const Submission &s) {
    os << s.simid << ",\t" << s.repid << ",\t"  << s.pubid << ",\t" << s.tnobs << ",\t" << s.group_;

    return os;
  }

  operator std::map<std::string, std::string>();
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
        "{}, {}, {}, {}, {}",
        s.simid, s.repid, s.pubid, s.tnobs, s.group_);
  }
};

#endif // SAMPP_SUBMISSION_H
