//===-- Policy.cpp - Policy Implementation --------------------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2021-01-19
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the implementation of Policy, PolicyChain, and
/// PolicyChainSet
///
//===----------------------------------------------------------------------===//

#include "Policy.h"

using namespace sam;

/// This mostly performs some string search, and decided what type of function
/// has been given as the input. Then, it uses a lua function template to
/// construct the appropriate function definition. Finally, it registers the
/// function to the given lua state.
///
/// @attention Since everything is happening via text processing, Policy is
/// quite sensitive to the function definition and it rejects anythings with
/// slightest of mistake in its definition.
///
/// @param[in]  p_def  The definition
/// @param      lua    The lua state
Policy::Policy(const std::string &p_def, sol::state &lua) {
  
  std::string f_def;

  if (p_def.find("min") != std::string::npos) {

    auto open_par = p_def.find('(');
    auto close_par = p_def.find(')');

    auto var_name = p_def.substr(open_par + 1, close_par - open_par - 1);

    auto f_name = fmt::format("min_{}", var_name);
    f_def = fmt::format(lua_temp_scripts["binary_function_template"],
                             f_name, var_name, var_name);

    lua.script(f_def);

    type = PolicyType::Min;
    func = lua[f_name];
    def = p_def;

  } else if (p_def.find("max") != std::string::npos) {

    auto open_par = p_def.find('(');
    auto close_par = p_def.find(')');

    auto var_name = p_def.substr(open_par + 1, close_par - open_par - 1);

    auto f_name = fmt::format("max_{}", var_name);
    f_def =
        fmt::format(lua_temp_scripts["binary_function_template"], f_name, var_name, var_name);

    lua.script(f_def);

    type = PolicyType::Max;
    func = lua[f_name];
    def = p_def;
  } else if (p_def[0] == '!') {

    std::string p_def_without_excl = p_def.substr(1);

    if (std::any_of(meta_variables.begin(), meta_variables.end(),
                    [&p_def_without_excl](auto &var) -> bool {
                      return p_def_without_excl == var;
                    })) {

      std::string f_name{"cond_not_" + p_def_without_excl};

      f_def = fmt::format(lua_temp_scripts["unary_function_template"],
                               f_name, p_def_without_excl + " == false");

      lua.script(f_def);

      type = PolicyType::Comp;
      func = lua[f_name];
      def = p_def;
    }

  } else if (std::any_of(
                 meta_variables.begin(), meta_variables.end(),
                 [&p_def](auto &var) -> bool { return p_def == var; })) {

    std::string f_name{"cond_" + p_def};

    f_def = fmt::format(lua_temp_scripts["unary_function_template"],
                             f_name, p_def + " == true");

    lua.script(f_def);

    type = PolicyType::Comp;
    func = lua[f_name];
    def = p_def;

  } else if (p_def.find("random") != std::string::npos) {

    type = PolicyType::Random;
    func = sol::function();
    def = "random";

  } else if (p_def.find("first") != std::string::npos) {

    std::string var_name{"id"};
    auto f_name = fmt::format("min_{}", var_name);
    f_def = fmt::format(lua_temp_scripts["binary_function_template"],
                             f_name, var_name, var_name);

    lua.script(f_def);

    type = PolicyType::First;
    func = lua[f_name];
    def = p_def;

  } else if (p_def.find("last") != std::string::npos) {

    std::string var_name{"id"};
    auto f_name = fmt::format("max_{}", var_name);
    f_def = fmt::format(lua_temp_scripts["binary_function_template"],
                             f_name, var_name, var_name);

    lua.script(f_def);

    type = PolicyType::Last;
    func = lua[f_name];
    def = p_def;

  } else if (p_def.find("all") != std::string::npos) {

    std::string var_name{"id"};
    auto f_name = fmt::format("all_{}", var_name);
    f_def = fmt::format("function all_id () return true end");

    lua.script(f_def);

    type = PolicyType::All;
    func = lua[f_name];
    def = p_def;

  } else if (std::any_of(cops.begin(), cops.end(), [&p_def](const auto &op) {
               return p_def.find(op.first) != std::string::npos;
             })) {
    // If none of the special functions are defined, then we might have a
    // comparison; so, we check for any of the binary operators, and constructs
    // a comparison function if necessary

    // Searching for the operator
    std::string s_op{};
    for (const auto &op : binary_operators) {
      if (p_def.find(op) != std::string::npos) {
        s_op = op;
        break;
      }
    }

    auto op_start = p_def.find(s_op);

    auto var_name = p_def.substr(0, op_start - 1);

    // Adding the actual value to the function name is not really necessary but
    // I want to make sure that I don't overwrite a function by accident
    auto f_name = fmt::format(
        "cond_{}", var_name + "_" + cops[s_op] + "_" +
                       p_def.substr(op_start + s_op.size() + 1, p_def.size()));

    // In case any of the logics has decimal points
    std::replace(f_name.begin(), f_name.end(), '.', '_');

    // In the case of any of the logics contains negative value
    std::replace(f_name.begin(), f_name.end(), '-', '_');

    f_def =
        fmt::format(lua_temp_scripts["unary_function_template"], f_name,
                    p_def); // Full text goes here

    // This is because lua uses ~ instead of !
    std::replace(f_def.begin(), f_def.end(), '!', '~');

    lua.script(f_def);

    type = PolicyType::Comp;
    func = lua[f_name];
    def = p_def;

  } else {
    throw std::invalid_argument("Invalid policy definition.");
  }
  
  spdlog::trace("Lua Function: {}", f_def);
}

/// This applies the current policy on a range of values, and returns a subset
/// of the range if it finds anything.
///
/// @param[in]  begin      The begin
/// @param      end        The end
///
/// @tparam     ForwardIt  This usually accepts Submission's ForwardIt
///
/// @return     Return a tuple containing three variables.
///
/// @attention  You can only use this on DependentVariable and Submission
/// objects, since these are the two classes that are registered as usertype.
template <typename ForwardIt>
std::tuple<bool, ForwardIt, ForwardIt>
Policy::operator()(const ForwardIt &begin, ForwardIt &end) {

  switch (type) {

    case PolicyType::Min: {
      auto it = std::min_element(begin, end, func);
      spdlog::trace("\n\t\tMin: {} \
                      \n\t\t\t{}",
                    def, *it);
      return {true, it, it};
    }

    case PolicyType::Max: {
      auto it = std::max_element(begin, end, func);
      spdlog::trace("\n\t\tMax: {} \
                      \n\t\t\t{}",
                    def, *it);
      return {true, it, it};
    }

    case PolicyType::Comp: {
      auto pit = std::partition(begin, end, func);
      spdlog::trace("\n\t\tComp: {} \
                      \n\t\t\t{}",
                    def, fmt::join(begin, end, "\n\t\t\t"));

      return {false, begin, pit};
    }

    case PolicyType::Random: {
      /// Shuffling the array and setting the end pointer to the first time,
      /// this basically mimic the process of selecting a random element from
      /// the list.
      Random::shuffle(begin, end);
      spdlog::trace("\n\t\tShuffled: {} \
                      \n\t\t\t{}",
                    def, fmt::join(begin, end, "\n\t\t\t"));
      return {true, begin, end};
    }

    case PolicyType::First: {

      spdlog::trace("\n\t\tFirst: {} \
                      \n\t\t\t{}",
                    def, *begin);

      return {true, begin, end};
    }

    case PolicyType::Last: {

      spdlog::trace("\n\t\tLast: {} \
                      \n\t\t\t{}",
                    def, *(end - 1));

      return {true, end - 1, end};
    }

    case PolicyType::All: {
      return {false, begin, end};
    }

    default: {
      throw std::invalid_argument("Invalid Policy Type.");
    }
  }
}

// std::optional<std::vector<DependentVariable>>
// Policy::operator()(Experiment *experiment) {
//  std::vector<DependentVariable> selection;
//
//  std::transform(experiment->dvs_.begin() + experiment->setup.nd(),
//                 experiment->dvs_.end(),
//                 selection.begin(),
//                 [&](auto &dv) -> bool {
//    return func(dv);
//  });
//
//  return selection;
//}

// std::optional<std::vector<Submission>>
// Policy::operator()(std::vector<Submission> &subs) {
//  std::vector<Submission> selection;
//
//  std::transform(subs.begin(), subs.end(),
//                 std::back_inserter(selection),
//                 [&](auto &s) -> bool {
//                    return func(s);
//                });
//
//  return selection;
//}

// -------------------------------------------------------- //
//                     Policy Chain                         //
// -------------------------------------------------------- //

PolicyChain::PolicyChain(const std::vector<std::string> &pchain_defs,
                         sol::state &lua) {

  
  for(int i{0}; i < pchain_defs.size(); ++i) {
    if (pchain_defs[i].empty()) {
      continue;;
    }
    pchain.emplace_back(pchain_defs[i], lua);
    
    // Checking whether there is any formula after the first noncomparitive formula
    if (pchain.back().type != PolicyType::Comp and i + 1 < pchain_defs.size())
      throw std::invalid_argument("No formula should be listed after any of the unary functions.");
  }
}

/// @param[in]  sub   The submission
///
/// @return     The result of applying all policies on the given submission
///
bool PolicyChain::operator()(const Submission &sub) {
  return std::all_of(pchain.begin(), pchain.end(),
                     [&](auto &policy) -> bool { return policy(sub); });
}

/// For every dependent variable, we check whether that dv satisfies any of the
/// given rules, if so, we set the verdict to `true` meaning that at least part
/// of the experiment satisfies all the policies. However, if after going
/// through all outcomes, none satisfies all the rules `false` will be returned,
/// meaning that none of the outcomes satisfied all the given rules
///
/// @note Currently this only uses `all_of` meaning that all policies need to be
/// satisfied for the check to be passed.
///
/// @todo I'm planning to implement the `logic` variable by which one can
/// control which logic is going to be used
///
/// @note This is horrible, there are two methods, one gets the pointer and
/// another one the ref. This is very confusing!
bool PolicyChain::operator()(Experiment *experiment) {

  spdlog::trace("Looking for {}", *this);

  bool verdict{false};
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {

    verdict |=
        std::all_of(pchain.begin(), pchain.end(), [&](auto &policy) -> bool {
          return policy(Submission{*experiment, i});
        });

    if (verdict) {
      return verdict;
    }
  }

  return verdict;
}

///
/// Applies the policy chain on the Experiment and returns a list of hits, or
/// returns an empty optional otherwise.
std::optional<std::vector<Submission>>
PolicyChain::operator()(Experiment &experiment) {

  spdlog::trace("Looking for {}", *this);

  std::vector<Submission> selections{};
  auto found_sth_unique{false};
  auto begin = experiment.dvs_.begin() + experiment.setup.nd();
  auto end = experiment.dvs_.end();

  // Looping through PolicyChain(s)
  for (auto &policy : pchain) {
    std::tie(found_sth_unique, begin, end) = policy(begin, end);

    if (found_sth_unique) {
      selections.emplace_back(experiment, begin->id_);
      spdlog::trace("✓ Found One!");
    }

    // One of the policies ended up with no results, so, we skip the
    // rest of the search
    if (begin == end) {
      break;
    }
  }

  if (found_sth_unique) {
    return selections;
  }

  // We found one!
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

std::optional<std::vector<Submission>>
PolicyChain::operator()(std::vector<Submission> &spool) {

  std::vector<Submission> selections{};
  auto found_sth_unique{false};
  auto begin = spool.begin();
  auto end = spool.end();

  for (auto &policy : pchain) {

    std::tie(found_sth_unique, begin, end) = policy(begin, end);

    if (found_sth_unique) {
      spdlog::trace("✓ Found something in the pile!");
      selections.push_back(*begin);
      return selections;
    }

    if (begin == end) {
      break;
    }
  }

  if (found_sth_unique) {
    return selections;
  }

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

// -------------------------------------------------------- //
//                    Policy Chain Set                      //
// -------------------------------------------------------- //

PolicyChainSet::PolicyChainSet(
    const std::vector<std::vector<std::string>> &psets_defs, sol::state &lua) {
  for (const auto &pchain_def : psets_defs) {
    PolicyChain pchain{pchain_def, lua};
    if (pchain.empty()) {
      continue;
    }
    pchains.emplace_back(pchain);
  }
}
