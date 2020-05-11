//
// Created by Amir Masoud Abdol on 2019-03-11.
//

#ifndef SAMPP_MAIN_H
#define SAMPP_MAIN_H

// see https://semver.org/
#define SAMPP_VERSION_MAJOR 0 // for incompatible API changes
#define SAMPP_VERSION_MINOR 1 // for backwards-compatible features
#define SAMPP_VERSION_PATCH 0 // for backwards-compatible bug fixes

//#include <mlpack/core.hpp>
//#include <mlpack/methods/neighbor_search/neighbor_search.hpp>
//#include <mlpack/methods/linear_regression/linear_regression.hpp>

namespace sam {
struct FLAGS {
  static bool PROGRESS;
  static bool DEBUG;
  static bool UPDATECONFIG;
};
} // namespace sam

#endif // SAMPP_MAIN_H
