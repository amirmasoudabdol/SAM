//
// Created by Amir Masoud Abdol on 03/04/2020.
//

///
/// @defgroup   Policies Policy-related Modules
/// @brief      List of available policy-related modules
///
/// Description to come!
///

#ifndef SAMPP_POLICY_H
#define SAMPP_POLICY_H

#include <ostream>
#include <fmt/core.h>
#include <fmt/format.h>

namespace sam {

enum class PolicyType : int { Min, Max, Comp, Random, First, Last, All };
enum class LogicType : int { AllOf, AnyOf, NoneOf };


NLOHMANN_JSON_SERIALIZE_ENUM(
LogicType,
{{LogicType::AllOf, "all_of"},
 {LogicType::AnyOf, "any_of"},
 {LogicType::NoneOf, "none_of"}})

/// @brief Implementation of the Policy.
///
/// A policy is a logical rule that it's being applied on an experiment, submission, or a set of submissions.
///
/// - In the case of submission, policy checks whether the given submission satisfies the given policy.
/// - In the case of set of submissions, or an experiment, policy checks whether the any of the items, ie.,
///   dvs or sub, will satisfy the given policy, if so, it'll return those items, otherwise, the output will be
///   empty.
///
///
/// @ingroup  Policies
///
struct Policy {
  PolicyType type;
  sol::function func;
  std::string def;

  Policy() = default;

  Policy(const std::string &p_def, sol::state &lua) {

    if (p_def.find("min") != std::string::npos) {

      auto open_par = p_def.find("(");
      auto close_par = p_def.find(")");

      auto var_name = p_def.substr(open_par + 1, close_par - open_par - 1);

      auto f_name = fmt::format("min_{}", var_name);
      auto f_def = fmt::format(lua_temp_scripts["min_script"], f_name, var_name,
                               var_name);

      lua.script(f_def);

      type = PolicyType::Min;
      func = lua[f_name];
      def = p_def;

    } else if (p_def.find("max") != std::string::npos) {

      auto open_par = p_def.find("(");
      auto close_par = p_def.find(")");

      auto var_name = p_def.substr(open_par + 1, close_par - open_par - 1);

      auto f_name = fmt::format("max_{}", var_name);
      auto f_def = fmt::format(lua_temp_scripts["max_script"], f_name, var_name,
                               var_name);

      lua.script(f_def);

      type = PolicyType::Max;
      func = lua[f_name];
      def = p_def;
    } else if (p_def.find("!sig") != std::string::npos) {

      auto f_name = "cond_not_sig";

      auto f_def =
          fmt::format(lua_temp_scripts["comp_script"], f_name, "sig == false");

      lua.script(f_def);

      type = PolicyType::Comp;
      func = lua[f_name];
      def = p_def;
      
    } else if (p_def.find("sig") != std::string::npos) {

      auto f_name = "cond_sig";

      auto f_def =
          fmt::format(lua_temp_scripts["comp_script"], f_name, "sig == true");

      lua.script(f_def);

      type = PolicyType::Comp;
      func = lua[f_name];
      def = p_def;
      
    } else if (p_def.find("random") != std::string::npos) {

      type = PolicyType::Random;
      func = sol::function();
      def = "random";

    } else if (p_def.find("first") != std::string::npos) {

      auto var_name = "id";
      auto f_name = fmt::format("min_{}", var_name);
      auto f_def = fmt::format(lua_temp_scripts["min_script"], f_name, var_name,
                               var_name);

      lua.script(f_def);

      type = PolicyType::First;
      func = lua[f_name];
      def = p_def;

    } else if (p_def.find("last") != std::string::npos) {

      auto var_name = "id";
      auto f_name = fmt::format("max_{}", var_name);
      auto f_def = fmt::format(lua_temp_scripts["min_script"], f_name, var_name,
                               var_name);

      lua.script(f_def);

      type = PolicyType::Last;
      func = lua[f_name];
      def = p_def;

    } else if (p_def.find("all") != std::string::npos) {

      auto var_name = "id";
      auto f_name = fmt::format("all_{}", var_name);
      auto f_def = fmt::format("function all_id () return true end");

      lua.script(f_def);

      type = PolicyType::All;
      func = lua[f_name];
      def = p_def;

    } else if (std::any_of(cops.begin(), cops.end(), [&p_def](const auto &op) {
                 return p_def.find(op.first) != std::string::npos;
               })) {
      // Found a comparision

      std::string s_op{};
      for (const auto &op : cops)
        if (p_def.find(op.first) != std::string::npos) {
          s_op = op.first;
          break;
        }

      auto op_start = p_def.find(s_op);

      auto var_name = p_def.substr(0, op_start - 1);

      // Adding the actual value to the function name is not really necessary but
      // I want to make sure that I don't overwrite a function by accident
      auto f_name =
          fmt::format("cond_{}", var_name + "_" + cops[s_op] + "_" +
                                     p_def.substr(op_start + s_op.size() + 1,
                                                  p_def.size()));

      // In case any of the logics has decimal points
      std::replace(f_name.begin(), f_name.end(), '.', '_');
      
      auto f_def = fmt::format(lua_temp_scripts["comp_script"], f_name,
                               p_def); // Full text goes here

      lua.script(f_def);

      type = PolicyType::Comp;
      func = lua[f_name];
      def = p_def;

    } else {
      throw std::invalid_argument("Invalid policy definition.");
    }
  }
  
  ///
  /// @brief      Given the forward iterator, it applies `func` on each item,
  /// and returns a subset of the range where all items satisfy the `func` criteria
  ///
  /// @param[in]  begin      The begin
  /// @param      end        The end
  ///
  /// @tparam     ForwardIt  This usually accepts Submission's ForwardIt
  ///
  /// @return     Return a tuple containing three variables.
  ///
  /// @note This can take care of either Group or Submission because the Lua instance is aware of them!
  ///
  template <typename ForwardIt>
  std::tuple<bool, ForwardIt, ForwardIt>
  operator()(const ForwardIt &begin, ForwardIt &end) {

    switch (type) {
        
      case PolicyType::Min: {
        auto it = std::min_element(begin, end, func);
        spdlog::trace("\n\t\tMin: {} \
                      \n\t\t\t{}", def, *it);
        return {true, it, it};
      } break;
        
      case PolicyType::Max: {
        auto it = std::max_element(begin, end, func);
        spdlog::trace("\n\t\tMax: {} \
                      \n\t\t\t{}", def, *it);
        return {true, it, it};
      } break;
        
      case PolicyType::Comp: {
        auto pit = std::partition(begin, end, func);
        spdlog::trace("\n\t\tComp: {} \
                       \n\t\t\t{}", def, fmt::join(begin, end, "\n\t\t\t"));
        
        return {false, begin, pit};
        
      } break;
        
      case PolicyType::Random: {
        /// Shuffling the array and setting the end pointer to the first time,
        /// this basically mimic the process of selecting a random element from
        /// the list.
        Random::shuffle(begin, end);
        spdlog::trace("\n\t\tShuffled: {} \
                      \n\t\t\t{}", def, fmt::join(begin, end, "\n\t\t\t"));
        return {true, begin, end};
        
      } break;
        
      case PolicyType::First: {
        
        // Sorting the groups based on their index
        //    std::sort(begin, end, func);
        
        spdlog::trace("\n\t\tFirst: {} \
                      \n\t\t\t{}", def, *begin);
        
        return {true, begin, end};
        
      } break;
        
      case PolicyType::Last: {
        
        // Sorting the groups based on their index
        //    std::sort(begin, end, func);
        
        spdlog::trace("\n\t\tLast: {} \
                      \n\t\t\t{}", def, *(end-1));
        
        return {true, end-1, end};
        
      } break;
        
      case PolicyType::All: {
        return {false, begin, end};
        
      } break;
        
      default: {
        throw std::invalid_argument("Invalid Policy Type.");
      }
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const Policy &policy) {
    os << policy.def;
    return os;
  }
  
  std::string operator()() {
    return def;
  }
  
  /// Returns the result of applying the policy on a submission
  bool operator()(const Submission& sub) {
    return func(sub);
  }
  
  
  std::optional<std::vector<Submission>> operator()(Experiment *experiment) {
    return {};
    /// @todo to be implemented
  }
  
  
  std::optional<std::vector<Submission>> operator()(std::vector<Submission> &subs) {
    return {};
    /// @todo to be implemented
  }

private:
  std::map<std::string, std::string> lua_temp_scripts{
    /// @todo these should be joined, they're technically the same!
      {"min_script", "function {} (l, r) return l.{} < r.{} end"},

      {"max_script", "function {} (l, r) return l.{} < r.{} end"},

      {"comp_script", "function {} (d) return d.{} end"}
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

/// Implementation of PolicyChain
///
/// PolicyChain is a list of Policies that will be executed chronologically. They
/// often being used to check whether an Experiment or a Submission can satisfy
/// the set of given rules.
///
/// @ingroup  Policies
///
struct PolicyChain {
  LogicType ltype = LogicType::AllOf;
  std::vector<std::string> defs;
  std::vector<Policy> pchain;

  PolicyChain() = default;

  PolicyChain(const std::vector<std::string> &pchain_defs, sol::state &lua) {

    for (auto &p_def : pchain_defs) {
      if (p_def.empty())
        continue;
      pchain.emplace_back(p_def, lua);
    }
  }
  
  PolicyChain(const std::vector<std::string> &pchain_defs, LogicType logic, sol::state &lua) {

    ltype = logic;
    
    for (auto &p_def : pchain_defs) {
      if (p_def.empty())
        continue;
      pchain.emplace_back(p_def, lua);
    }
  }

  Policy &operator[](std::size_t idx) { return pchain[idx]; }
  const Policy &operator[](std::size_t idx) const { return pchain[idx]; }

  auto begin() { return pchain.begin(); };
  auto cbegin() const { return pchain.cbegin(); };
  auto end() { return pchain.end(); };
  auto cend() const { return pchain.cend(); };
  
  
  ///
  /// @brief      Checks whether the given Submission satisfies __all__ listed policies.
  ///
  /// @param[in]  sub   The submission
  ///
  /// @return     The result of applying all policies on the given submission
  ///
  bool operator()(const Submission &sub) {
    return std::all_of(pchain.begin(), pchain.end(), [&](auto &policy) -> bool {
      return policy(sub);
    });
  }
  
  /// @brief  Determines whether the experiment satisfies any of the given policies
  ///
  /// For every dependent variable, we check whether that dv satisfies any of the given rules,
  /// if so, we set the verdict to `true` meaning that at least part of the experiment satisfies
  /// all the policies. However, if after going through all outcomes, none satisfies all the rules
  /// `false` will be returned, meaning that none of the outcomes satisfied all the given rules
  ///
  /// @note Currently this only uses `all_of` meaning that all policies need to be satisfied for
  /// the check to be passed.
  ///
  /// @todo I'm planning to implement the `logic` variable by which one can control which logic
  /// is going to be used
  ///
  /// @note This is horrible, there are two methods, one gets the pointer and another one the ref.
  /// This is very confusing!
  bool operator()(Experiment *experiment) {
    
    spdlog::trace("Looking for {}", *this);
    
    bool verdict {false};
    for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
         ++i, ++d %= experiment->setup.nd()) {
      
      verdict |= std::all_of(pchain.begin(), pchain.end(), [&](auto &policy) -> bool {
        return policy(Submission{*experiment, i});
      });
      
      if (verdict)
        return verdict;
      
    }
    
    return verdict;
  }
  
  /// @brief  Returns a list of submission satisfying the policy chain
  ///
  /// Applies the policy chain on the Experiment and returns a list of hits, or
  /// returns an empty optional otherwise.
  std::optional<std::vector<Submission>> operator()(Experiment &experiment) {
    
    spdlog::trace("Looking for {}", *this);
    
    std::vector<Submission> selections {};
    auto found_sth_unique {false};
    auto begin = experiment.groups_.begin() + experiment.setup.nd();
    auto end = experiment.groups_.end();
    
    /// Looping through PolicyChain(s)
    for (auto &policy : pchain) {
      std::tie(found_sth_unique, begin, end) = policy(begin, end);
      
      if (found_sth_unique) {
        selections.emplace_back(experiment, begin->id_);
        spdlog::trace("✓ Found One!");
      }
      
      // One of the policies ended up with no results, so, we skip the
      // rest of the search
      if (begin == end)
        break;
      
    }
    
    if (found_sth_unique)
      return selections;
    
    /// We found one!
    if (begin + 1 == end) {
      selections.emplace_back(experiment, begin->id_);
      spdlog::trace("✓ Found the only one!");
      return selections;
    } else if (begin != end) { /// We found a bunch
      
      for (auto it{begin}; it != end; ++it) {
        selections.emplace_back(experiment, it->id_);
      }
      spdlog::trace("✓ Found a bunch: {}", selections);


      return selections;
    }

    spdlog::trace("✗ Found nothing!");
    return {};
  }
  
  std::optional<std::vector<Submission>> operator()(std::vector<Submission> &spool) {
    
    std::vector<Submission> selections {};
    auto found_sth_unique {false};
    auto begin = spool.begin();
    auto end = spool.end();
    
    for (auto &policy : pchain) {
      
      std::tie(found_sth_unique, begin, end) =
      policy(begin, end);
      
      if (found_sth_unique) {
        spdlog::trace("✓ Found something in the pile!");
        selections.push_back(*begin);
        return selections;
      }
      
      if (begin == end)
        break;
    }
    
    if (found_sth_unique)
      return selections;
    
    if (begin + 1 == end) {
      spdlog::trace("✓ Found the only one!");
      selections.push_back(*begin);
      return selections;
    } else if (begin != end) { 
      for (auto it{begin}; it != end; ++it) {
        selections.push_back(*it);
        spdlog::trace("\t {}", *it);
      }
      return selections;
    }
    
    spdlog::trace("✗ Found nothing!");
    return {};
    
  }
  
  bool empty() const { return pchain.empty(); };
};

inline void to_json(json &j, const PolicyChain &p) {
  j = json{
    {"definitions", p.defs},
    {"logic", p.ltype}
  };
}

inline void from_json(const json &j, PolicyChain &p, sol::state &lua) {
  p = PolicyChain(j.at("definitions"), j.at("logic"), lua);
}

/// PolicyChainSet is a collection of PolicyChains
///
/// They are mainly being used and interpreted like a list of preferences, and will be
/// executed chronologically. Most function will go through the list one by one, and will
/// quit as soon as one of the PolicyChains find at least on outcome from a given list of
/// options, e.g., SubmissionPool or Experiment.
///
/// @ingroup  Policies
///
struct PolicyChainSet {
  std::vector<PolicyChain> pchains;

  PolicyChainSet() = default;

  PolicyChainSet(const std::vector<std::vector<std::string>> &psets_defs,
                 sol::state &lua) {
    for (auto &pchain_def : psets_defs) {
      PolicyChain pchain{pchain_def, lua};
      if (pchain.empty())
        continue;
      pchains.emplace_back(pchain);
    }
  }

  auto begin() { return pchains.begin(); };
  auto cbegin() const { return pchains.cbegin(); };
  auto end() { return pchains.end(); };
  auto cend() const { return pchains.cend(); };
  
  std::optional<Submission> operator()(const Experiment *expr) {
    /// @todo: Check and return,
    
    return Submission{};
  }

  PolicyChain &operator[](std::size_t idx) { return pchains[idx]; }
  const PolicyChain &operator[](std::size_t idx) const { return pchains[idx]; }
  
  size_t size() const { return pchains.size(); };
  bool empty() const { return pchains.empty(); };
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
