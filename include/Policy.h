//===-- Policy.h - Policy Deceleration ------------------------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 03/04/2020.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the deceleration of Policy families, i.e., Policy,
/// PolicyChain, PolicyChainSet
///
//===----------------------------------------------------------------------===//
///
/// @defgroup   Policies Policy-related Modules
/// @brief      List of available policy-related modules
///

#ifndef SAMPP_POLICY_H
#define SAMPP_POLICY_H

#include <fmt/core.h>
#include <fmt/format.h>
#include <ostream>
#include <string>

#include "Experiment.h"
#include "Submission.h"
#include "sam.h"
#include "sol/sol.hpp"

namespace sam {

/// @brief      Indicates the type of the Policy
///
/// This is mainly being used by Policy to decide which type of formula it's
/// dealing with.
///
/// @ingroup    Policies
enum class PolicyType : int { Min, Max, Comp, Random, First, Last, All };

/// @brief      Indicates the type of the PolicyChain
///
/// A PolicyChain can either be used to perform *selection*, or a *decision*.
/// The main difference between them being that the `::Selection` chains could
/// include a function call, e.g., `min`, while the `::Decision` chains cannot.
///
/// @ingroup    Policies
enum class PolicyChainType : int { Selection, Decision };

/** @name Handy Policy Typedefs
 *
 */
///@{
using PolicyDefinition = std::string;
using PolicyChainDefinition = std::vector<std::string>;
using PolicyChainSetDefinition = std::vector<std::vector<std::string>>;
///@}

/// @brief Implementation of the Policy class.
///
/// A policy is a logical rule that it's being applied on an experiment,
/// submission, or a set of submissions. A policy can be used to perform two
/// type of operation on either of the mentioned data structures, selection or
/// decision.
///
/// - To check whether a submission, a dependent variable satisfies a given
/// Policy, you must use the appropriate call operator in the form of `bool
/// operator()`.
/// - To filter a list of submissions or dependent variables of the experiment
/// based on the given Policy, you must use the iterator-based operator.
///
/// @ingroup  Policies
struct Policy {
  PolicyType type;
  PolicyDefinition def;
  sol::function func;

  Policy() = default;

  /// Creates a policy, and registers it to the available lua state
  Policy(const std::string &p_def, sol::state &lua);

  /// Filters the range based on the given policy
  template <typename ForwardIt>
  std::optional<std::pair<ForwardIt, ForwardIt>> operator()(ForwardIt begin,
                                                            ForwardIt end);

  /// Returns the result of applying the policy on a submission
  [[nodiscard]] bool operator()(const Submission &sub) const {
    return func(sub);
  }

  /// Returns the result of applying the policy on a dependent variable
  [[nodiscard]] bool operator()(const DependentVariable &dv) const {
    return func(dv);
  }
  [[nodiscard]] bool operator()(DependentVariable &dv) const {
    return func(dv);
  }

  /// String operator for the JSON library
  explicit operator std::string() const { return def; }

private:
  std::map<std::string, std::string> lua_temp_scripts{
      {"binary_function_template", "function {} (l, r) return l.{} < r.{} end"},

      {"unary_function_template", "function {} (d) return d.{} end"}};

  std::map<std::string, std::string> cops = {
      {">=", "greater_eq"}, {"<=", "lesser_eq"}, {">", "greater"},
      {"<", "lesser"},      {"==", "equal"},     {"!=", "not_equal"}};

  std::vector<std::string> quantitative_variables{"id", "nobs", "mean",
                                                  "pvalue", "effect"};

  std::vector<std::string> meta_variables{"sig", "hacked", "candidate"};

  std::vector<std::string> binary_operators{">=", "<=", "<", ">", "==", "!="};

  std::vector<std::string> unary_functions{"min",   "max",  "random",
                                           "first", "last", "all"};
};

inline void to_json(json &j, const Policy &p) {
  j = json{{"definition", p.def}};
}

inline void from_json(const json &j, Policy &p, sol::state &lua) {
  p = Policy(j.at("definition"), lua);
}

/// @brief  Implementation of the PolicyChain class
///
/// PolicyChains are a list of Policies that will be executed chronologically.
/// They are often being used to check whether an Experiment or a Submission can
/// satisfy all the given policies.
///
/// PolicyChains can be defined in two different ways. They are either a
/// _selection_ or _decision_ chains. The `::Selection` chains are used to
/// filter an experiment or a list of submissions based on the given policies.
/// The `::Decision` chains are used to check whether any of the submissions or
/// dependent variables are satisfying all the available policies in the given
/// chain.
///
/// @ingroup  Policies
struct PolicyChain {
  PolicyChainType type_;
  PolicyChainDefinition defs;
  std::vector<Policy> pchain;

  PolicyChain() = default;

  /// PolicyChain constructor
  PolicyChain(const PolicyChainDefinition &pchain_defs, PolicyChainType type,
              sol::state &lua);

  /// Checks whether the given Submission satisfies __all__ all policies.
  [[nodiscard]] bool operator()(const Submission &sub);

  /// Checks whether the given DependentVariable satisfies __all__ the policies.
  [[nodiscard]] bool operator() (const DependentVariable &dv);
  [[nodiscard]] bool operator() (DependentVariable &dv);

  /// Determines whether the experiment satisfies any of the given policies
  [[nodiscard]] bool operator()(Experiment *experiment);

  /// Returns a list of submission satisfying the policy chain
  std::optional<std::vector<Submission>> operator()(Experiment &experiment);

  /// Returns a list of submissions satisfying the policy chain
  std::optional<std::vector<Submission>>
  operator()(std::vector<Submission> &spool);

  /** @name STL-like operators
   *
   *  List of STL-like operators for ease of use and comparability purposes
   */
  ///@{
  Policy &operator[](std::size_t idx) { return pchain[idx]; }
  const Policy &operator[](std::size_t idx) const { return pchain[idx]; }

  [[nodiscard]] auto begin() { return pchain.begin(); };
  [[nodiscard]] auto cbegin() const { return pchain.cbegin(); };
  [[nodiscard]] auto end() { return pchain.end(); };
  [[nodiscard]] auto cend() const { return pchain.cend(); };

  [[nodiscard]] bool empty() const { return pchain.empty(); };
  ///@}
};

inline void to_json(json &j, const PolicyChain &p) {
  j = json{{"definitions", p.defs}};
}

inline void from_json(const json &j, PolicyChain &p, sol::state &lua) {
  p = PolicyChain(j.at("definitions"), PolicyChainType::Decision, lua);
}

/// PolicyChainSet is a collection of PolicyChains
///
/// They are mainly being used and interpreted like a list of preferences, and
/// will be executed chronologically. Most function will go through the list one
/// by one, and will quit as soon as one of the PolicyChains find at least on
/// outcome from a given list of options, e.g., SubmissionPool or Experiment.
///
/// @ingroup  Policies
struct PolicyChainSet {
  std::vector<PolicyChain> pchains;

  PolicyChainSet() = default;

  /// PolicyChainSet constructor
  PolicyChainSet(const PolicyChainSetDefinition &psets_defs, sol::state &lua);

  /// Returns a list of submissions from DVs of an experiment
  std::optional<std::vector<Submission>> operator()(Experiment &expr);

  /// Returns a list of submissions from a list of submissions
  std::optional<std::vector<Submission>>
  operator()(std::vector<Submission> &spool);

  /** @name STL-like operators
   *
   *  List of STL-like operators for ease of use and comparability purposes
   */
  ///@{
  PolicyChain &operator[](std::size_t idx) { return pchains[idx]; }
  const PolicyChain &operator[](std::size_t idx) const { return pchains[idx]; }

  [[nodiscard]] auto begin() { return pchains.begin(); };
  [[nodiscard]] auto cbegin() const { return pchains.cbegin(); };
  [[nodiscard]] auto end() { return pchains.end(); };
  [[nodiscard]] auto cend() const { return pchains.cend(); };

  [[nodiscard]] size_t size() const { return pchains.size(); };
  [[nodiscard]] bool empty() const { return pchains.empty(); };
  ///@}
};

} // namespace sam

namespace fmt {
template <> struct formatter<sam::Policy> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  };

  template <typename FormatContext>
  auto format(sam::Policy const &policy, FormatContext &ctx) {
    return format_to(ctx.out(), "{}", policy.def);
  };
};

template <> struct formatter<sam::PolicyChain> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  };

  template <typename FormatContext>
  auto format(sam::PolicyChain const &pchain, FormatContext &ctx) {
    return format_to(ctx.out(), "[{}]",
                     join(pchain.cbegin(), pchain.cend(), ", "));
  };
};
} // namespace fmt

#endif // SAMPP_POLICY_H
