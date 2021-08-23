//
// Created by Amir Masoud Abdol on 2019-03-11.
//

#ifndef SAMPP_MAIN_H
#define SAMPP_MAIN_H

// see https://semver.org/
#define SAMPP_VERSION_MAJOR 0 // for incompatible API changes
#define SAMPP_VERSION_MINOR 1 // for backwards-compatible features
#define SAMPP_VERSION_PATCH 0 // for backwards-compatible bug fixes

#undef JSON_DIAGNOSTICS
#define JSON_DIAGNOSTICS 1

#include <mlpack/core.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::ordered_json;

namespace sam {

enum class LogLevel : int
{
  trace = 0,
  debug,
  info,
  warn,
  err,
  critical,
  off,
  n_levels
};

NLOHMANN_JSON_SERIALIZE_ENUM(
LogLevel,
{
  {LogLevel::trace, "trace"},
  {LogLevel::debug, "debug"},
  {LogLevel::info, "info"},
  {LogLevel::warn, "warn"},
  {LogLevel::err, "err"},
  {LogLevel::critical, "critical"},
  {LogLevel::off, "off"}
})


template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

} // namespace sam

#endif // SAMPP_MAIN_H
