//
// Created by Amir Masoud Abdol on 03/04/2020.
//

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
  
  template <typename ForwardIt>
  std::tuple<bool, ForwardIt, ForwardIt>
  operator()(const ForwardIt &begin, ForwardIt &end) {
      switch (type) {

      case PolicyType::Min: {
        auto it = std::min_element(begin, end, func);
        spdlog::debug("Min: {}", def);
        spdlog::debug("\t {}", *it);
        return {true, it, it};
      } break;

      case PolicyType::Max: {
        auto it = std::max_element(begin, end, func);
        spdlog::debug("Max: {}", def);
        spdlog::debug("\t {}", *it);
        return {true, it, it};
      } break;

      case PolicyType::Comp: {
        auto pit = std::partition(begin, end, func);
        spdlog::debug("Comp: {}", def);
        for (auto it{begin}; it != pit; ++it) {
          spdlog::debug("\t {}", *it);
        }

        return {false, begin, pit};

      } break;

      case PolicyType::Random: {
        /// Shuffling the array and setting the end pointer to the first time,
        /// this basically mimic the process of selecting a random element from
        /// the list.
        Random::shuffle(begin, end);
        spdlog::debug("Shuffled: {}", def);
        for (auto it{begin}; it != end; ++it) {
          spdlog::debug("\t {}", *it);
        }
        return {true, begin, end};

      } break;

      case PolicyType::First: {
        /// This strictly returns a pointer to the __first__ element of a
        /// given container

        spdlog::debug("First: {}", def);
        spdlog::debug("\t {}", *begin);

        return {true, begin, end};

      } break;
          
      case PolicyType::Last: {
        /// This strictly returns a pointer to the __last__ element of a
        /// given container

          spdlog::debug("Last: {}", def);
          spdlog::debug("\t {}", *end);

          return {true, end, end};

      } break;

      case PolicyType::All: {
        return {false, begin, end};

      } break;
      }
  }

  friend std::ostream &operator<<(std::ostream &os, const Policy &policy) {
    os << policy.def;
    return os;
  }
  
  bool operator()(const Submission& sub) {
    return func(sub);
  }

private:
  std::map<std::string, std::string> lua_temp_scripts{
    /// \todo these should be joined, they're technically the same!
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
  
  bool operator()(const Submission &sub) {
    /// \todo: we check if we comply with the rules
    return false;
  }
  
  /// @brief  Determine whether the experiment satisfies any of the given policies
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
  bool operator()(Experiment *experiment) {
    
    bool verdict {false};
    for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
         ++i, ++d %= experiment->setup.nd()) {
      
      verdict |= std::all_of(pchain.begin(), pchain.end(), [&](auto &policy) -> bool {
        return policy.func(Submission{*experiment, i});
      });
      
      if (verdict)
        return verdict;
      
    }
    
    return verdict;
  }
  
  /// \note this is probably a better implementation but for now, I'll stay with the one above
  /// to get around the stopping condition implementation
//  std::optional<Submission> operator()(const Experiment *expr) {
//    /// \todo: we check the experiment, and return the submission
//
//    return Submission{};
//  }

  friend std::ostream &operator<<(std::ostream &os, const PolicyChain &chain) {
    os << "[";
//    fmt::print(os, "{}", fmt::join(chain.cbegin(), chain.cend(), ", "));
    os << "]";
    return os;
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
    /// \todo: Check and return,
    
    return Submission{};
  }

  friend std::ostream &operator<<(std::ostream &os, const PolicyChainSet &set) {
//    fmt::print(os, "[{}]", fmt::join(set.cbegin(), set.cend(), ", "));
    return os;
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
