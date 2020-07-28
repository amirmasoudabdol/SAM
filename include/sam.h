//
// Created by Amir Masoud Abdol on 2019-03-11.
//

#ifndef SAMPP_MAIN_H
#define SAMPP_MAIN_H

// see https://semver.org/
#define SAMPP_VERSION_MAJOR 0 // for incompatible API changes
#define SAMPP_VERSION_MINOR 1 // for backwards-compatible features
#define SAMPP_VERSION_PATCH 0 // for backwards-compatible bug fixes

#include <mlpack/core.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace sam {
struct FLAGS {
  static bool PROGRESS;
  static bool DEBUG;
  static bool UPDATECONFIG;
};

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

} // namespace sam

#endif // SAMPP_MAIN_H
