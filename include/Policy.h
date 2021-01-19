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

#include <ostream>
#include <fmt/core.h>
#include <fmt/format.h>

#include "sam.h"
#include "sol/sol.hpp"
#include "Experiment.h"
#include "Submission.h"

namespace sam {

enum class PolicyType : int { Min, Max, Comp, Random, First, Last, All };


/// @brief Implementation of the Policy class.
///
/// A policy is a logical rule that it's being applied on an experiment, submission, or a
///  set of submissions.
///
/// - In the case of submission, policy checks whether the given submission satisfies
/// the given policy.
/// - In the case of set of submissions, or an experiment, policy checks whether the any
///  of the items, ie., dvs or subs, will satisfy the given policy, if so, it'll return
///  those items, otherwise, the output will be empty.
///
/// @ingroup  Policies
struct Policy {
  PolicyType type;
  sol::function func;
  std::string def;

  Policy() = default;

  /// Creates a policy, and registers it to the available lua state
  Policy(const std::string &p_def, sol::state &lua);
  
  /// Given the forward iterator, it applies `func` on each item,
  /// and returns a subset of the range where all items satisfy the `func` criteria
  template <typename ForwardIt>
  std::tuple<bool, ForwardIt, ForwardIt>
  operator()(const ForwardIt &begin, ForwardIt &end);
    
  /// Returns the result of applying the policy on a submission
  [[nodiscard]] bool operator()(const Submission& sub) const {
    return func(sub);
  }
  
  /// Returns the result of applying the policy on a dependent variable
//  bool operator()(const DependentVariable &dv) {
//    return func(dv);
//  }
  
  /// Returns a list of dependent variables that are satisfying the policy
  std::optional<std::vector<DependentVariable>> operator()(Experiment *experiment);
  
  /// Returns a list of submissions that are satisfying the policy
  std::optional<std::vector<Submission>> operator()(std::vector<Submission> &subs);
  
  /// String operator for the JSON library
  operator std::string() const {
    return def;
  }

private:
  std::map<std::string, std::string> lua_temp_scripts{
      {"binary_function_template", "function {} (l, r) return l.{} < r.{} end"},

      {"unary_function_template", "function {} (d) return d.{} end"}
      };

  std::map<std::string, std::string> cops = {
      {">=", "greater_eq"}, {"<=", "lesser_eq"}, {">", "greater"},
      {"<", "lesser"},      {"==", "equal"},     {"!=", "not_equal"}};
};

inline void to_json(json &j, const Policy &p) {
  j = json{{"definition", p.def}};
}

inline void from_json(const json &j, Policy &p, sol::state &lua) {
  p = Policy(j.at("definition"), lua);
}

/// Implementation of the PolicyChain class
///
/// PolicyChain is a list of Policies that will be executed chronologically. They
/// often being used to check whether an Experiment or a Submission can satisfy
/// the set of given rules.
///
/// @ingroup  Policies
struct PolicyChain {
  std::vector<std::string> defs;
  std::vector<Policy> pchain;

  PolicyChain() = default;

  /// PolicyChain constructor
  PolicyChain(const std::vector<std::string> &pchain_defs, sol::state &lua);
  
  /// Checks whether the given Submission satisfies __all__ listed policies.
  bool operator()(const Submission &sub);
  
  /// Determines whether the experiment satisfies any of the given policies
  bool operator()(Experiment *experiment);
  
  /// Returns a list of submission satisfying the policy chain
  std::optional<std::vector<Submission>> operator()(Experiment &experiment);
  
  /// Returns a list of submissions satisfying the policy chain
  std::optional<std::vector<Submission>> operator()(std::vector<Submission> &spool);
  
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
  j = json{
    {"definitions", p.defs}
  };
}

inline void from_json(const json &j, PolicyChain &p, sol::state &lua) {
  p = PolicyChain(j.at("definitions"), lua);
}

/// PolicyChainSet is a collection of PolicyChains
///
/// They are mainly being used and interpreted like a list of preferences, and will be
/// executed chronologically. Most function will go through the list one by one, and will
/// quit as soon as one of the PolicyChains find at least on outcome from a given list of
/// options, e.g., SubmissionPool or Experiment.
///
/// @ingroup  Policies
struct PolicyChainSet {
  std::vector<PolicyChain> pchains;

  PolicyChainSet() = default;

  PolicyChainSet(const std::vector<std::vector<std::string>> &psets_defs,
                 sol::state &lua);
  
  
  /// Not Implemented Yet!
//  std::optional<std::vector<DependentVariable>> operator()(const Experiment *expr);

  /// Not Implemented Yet!
//  std::optional<std::vector<Submission>> operator()(const std::vector<Submission> &subs);

  
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
  template<>
  struct formatter<sam::Policy>
  {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    };

    template<typename FormatContext>
    auto format(sam::Policy const& policy, FormatContext& ctx) {
      return format_to(ctx.out(), "{}", policy.def);
    };
  };

  template<>
  struct formatter<sam::PolicyChain>
  {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    };

    template<typename FormatContext>
    auto format(sam::PolicyChain const& pchain, FormatContext& ctx) {
      return format_to(ctx.out(), "[{}]", join(pchain.cbegin(), pchain.cend(), ", "));
    };
  };
} // namespace fmt

#endif // SAMPP_POLICY_H
