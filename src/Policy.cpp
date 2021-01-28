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

///
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
///
Policy::Policy(const std::string &p_def, sol::state &lua) {

  std::string f_def;

  if (p_def.find("min") != std::string::npos) {

    auto open_par = p_def.find('(');
    auto close_par = p_def.find(')');

    auto var_name = p_def.substr(open_par + 1, close_par - open_par - 1);

    auto f_name = fmt::format("min_{}", var_name);
    f_def = fmt::format(lua_temp_scripts["binary_function_template"], f_name,
                        var_name, var_name);

    lua.script(f_def);

    type = PolicyType::Min;
    func = lua[f_name];
    def = p_def;

  } else if (p_def.find("max") != std::string::npos) {

    auto open_par = p_def.find('(');
    auto close_par = p_def.find(')');

    auto var_name = p_def.substr(open_par + 1, close_par - open_par - 1);

    auto f_name = fmt::format("max_{}", var_name);
    f_def = fmt::format(lua_temp_scripts["binary_function_template"], f_name,
                        var_name, var_name);

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

      f_def = fmt::format(lua_temp_scripts["unary_function_template"], f_name,
                          p_def_without_excl + " == false");

      lua.script(f_def);

      type = PolicyType::Comp;
      func = lua[f_name];
      def = p_def;
    }

  } else if (std::any_of(
                 meta_variables.begin(), meta_variables.end(),
                 [&p_def](auto &var) -> bool { return p_def == var; })) {

    std::string f_name{"cond_" + p_def};

    f_def = fmt::format(lua_temp_scripts["unary_function_template"], f_name,
                        p_def + " == true");

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
    f_def = fmt::format(lua_temp_scripts["binary_function_template"], f_name,
                        var_name, var_name);

    lua.script(f_def);

    type = PolicyType::First;
    func = lua[f_name];
    def = p_def;

  } else if (p_def.find("last") != std::string::npos) {

    std::string var_name{"id"};
    auto f_name = fmt::format("max_{}", var_name);
    f_def = fmt::format(lua_temp_scripts["binary_function_template"], f_name,
                        var_name, var_name);

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

    f_def = fmt::format(lua_temp_scripts["unary_function_template"], f_name,
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

///
/// This applies the current policy on a range of values, and returns a subset
/// of the range if it finds anything. If not, it will return an empty optional.
///
/// @attention  You can only use this on DependentVariable and Submission
/// objects, since these are the two classes that are registered as lua
/// usertype.
///
/// @param[in]  begin      The begin
/// @param      end        The end
///
/// @tparam     ForwardIt  This usually accepts Submission's ForwardIt
///
/// @return     Return a tuple containing three variables.
///
template <typename ForwardIt>
std::optional<std::pair<ForwardIt, ForwardIt>>
Policy::operator()(ForwardIt begin, ForwardIt end) {

  switch (type) {

  // This is probably the most used one!
  case PolicyType::Comp: {
    auto pit = std::partition(begin, end, func);
    spdlog::trace("\n\t\tComp: {} \
                    \n\t\t\t{}",
                  def, fmt::join(begin, pit, "\n\t\t\t"));

    end = pit;
  } break;

  // This is somewhat a special case where I can allow it in between but then it
  // doesn't fulfill any purpose anyway
  case PolicyType::All: {
    spdlog::trace("\n\t\tFunc: {} \
                    \n\t\t\t{}",
                  def, fmt::join(begin, end, "\n\t\t\t"));
    // We don't need to do anything here...
  } break;

  case PolicyType::Min: {
    auto it = std::min_element(begin, end, func);
    spdlog::trace("\n\t\tFunc: {} \
                      \n\t\t\t{}",
                  def, *it);
    begin = it;
    end = it + 1;
  } break;

  case PolicyType::Max: {
    auto it = std::max_element(begin, end, func);
    spdlog::trace("\n\t\tFunc: {} \
                      \n\t\t\t{}",
                  def, *it);
    begin = it;
    end = it + 1;
  } break;

  case PolicyType::Random: {
    /// Shuffling the array and setting the end pointer to the first time,
    /// this basically mimic the process of selecting a random element from
    /// the list.
    Random::shuffle(begin, end);
    spdlog::trace("\n\t\tFunc: {} \
                      \n\t\t\t{}",
                  def, fmt::join(begin, end, "\n\t\t\t"));
    end = begin + 1;
  } break;

  case PolicyType::First: {

    spdlog::trace("\n\t\tFunc: {} \
                      \n\t\t\t{}",
                  def, *begin);

    end = begin + 1;
  } break;

  case PolicyType::Last: {

    spdlog::trace("\n\t\tFunc: {} \
                      \n\t\t\t{}",
                  def, *(end - 1));
    begin = end - 1;
    end = begin + 1;
  } break;

  default: {
    throw std::invalid_argument("Invalid Policy Type.");
  }
  }

  if (begin == end) {
    return {};
  }

  return std::make_pair(begin, end);
}

// -------------------------------------------------------- //
//                     Policy Chain                         //
// -------------------------------------------------------- //

///
/// It constructs a PolicyChain object, and it also takes care of a few other
/// things like making sure that no comparison operator comes after any of the
/// function calls. It also sets the type of the PolicyChain, as described in
/// PolicyChainType.
///
PolicyChain::PolicyChain(const std::vector<std::string> &pchain_defs,
                         PolicyChainType type, sol::state &lua)
    : type_{type} {

  for (int i{0}; i < pchain_defs.size(); ++i) {

    if (pchain_defs[i].empty()) {
      continue;
    }
    pchain.emplace_back(pchain_defs[i], lua);
    
    if (type_ == PolicyChainType::Decision and
        pchain.back().type != PolicyType::Comp) {
      throw std::domain_error("Decision policies could only contain comparative operators;");
    }

    // Checking whether there is any formula after the first non-comparative
    // formula
    if (pchain.back().type != PolicyType::Comp and i + 1 < pchain_defs.size()) {
      throw std::invalid_argument(
          "No formula should be listed after any of the unary functions.");
    }
  }
}

///
/// @param[in]  sub   The submission
///
/// @return     The result of applying all policies on the given submission
///
bool PolicyChain::operator()(const Submission &sub) {
  return std::all_of(pchain.begin(), pchain.end(),
                     [&](auto &policy) -> bool { return policy(sub); });
}

///
/// @param[in]  dv   The dependent variable
///
/// @return     The result of applying all policies on the given dv
///
bool PolicyChain::operator()(const DependentVariable &dv) {
  return std::all_of(pchain.begin(), pchain.end(),
                     [&](auto &policy) -> bool { return policy(dv); });
}

bool PolicyChain::operator()(DependentVariable &dv) {
  return std::all_of(pchain.begin(), pchain.end(),
                     [&](auto &policy) -> bool { return policy(dv); });
}

///
/// This checks whether any of the DependentVariable(s) are satisfying all the
/// policies of the chain.
///
/// @todo Refactor this such that it accepts a reference
///
/// @return     Returns `true` if at least one DV satisfies all the policies,
/// otherwise `false`.
///
bool PolicyChain::operator()(Experiment *experiment) {

  spdlog::trace("Looking for {}", *this);

  bool verdict{false};
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {

    // It basically repeatedly calls the method above!
    verdict |= (*this)(experiment->dvs_[i]);

    if (verdict) {
      return verdict;
    }
  }

  return false;
}

///
/// This applies the policy chain on the Experiment and returns a list of
/// submissions (constructed from dependent variables of the experiment) that
/// are satisfying all the available policies.
///
/// @param      expr  The experiment
///
/// @return     An optional list of submissions
///
std::optional<std::vector<Submission>>
PolicyChain::operator()(Experiment &experiment) {

  spdlog::trace("Looking for {}", *this);

  std::vector<Submission> selections{};
  auto begin = experiment.dvs_.begin() + experiment.setup.nd();
  auto end = experiment.dvs_.end();

  // Looping through PolicyChain(s)
  for (auto &policy : pchain) {
    auto res = policy(begin, end);

    if (res) {
      begin = res->first;
      end = res->second;
    } else {
      return std::nullopt;
    }
  }

  if (begin != end) {
    for (auto it{begin}; it != end; ++it) {
      selections.emplace_back(experiment, it->id_);
    }
    spdlog::trace("✓ Found a bunch: {}", selections);
    return selections;
  }

  spdlog::trace("✗ Found nothing!");
  return std::nullopt;
}

///
/// This applies all the available policies of the chain on the given pool of
/// submissions and if there were any hit, it returns those. If not, it will
/// report an empty list.
///
/// @param      spool  The list of submissions
///
/// @return     A subset of `spool`, if any
///
std::optional<std::vector<Submission>>
PolicyChain::operator()(std::vector<Submission> &spool) {

  std::vector<Submission> selections{};
  auto begin = spool.begin();
  auto end = spool.end();

  for (auto &policy : pchain) {

    auto res = policy(begin, end);

    if (res) {
      begin = res->first;
      end = res->second;
    } else {
      return std::nullopt;
    }
  }

  if (begin != end) {
    for (auto it{begin}; it != end; ++it) {
      selections.push_back(*it);
      spdlog::trace("\t {}", *it);
    }
    spdlog::trace("✓ Found a bunch: {}", selections);
    return selections;
  }

  spdlog::trace("✗ Found nothing!");
  return std::nullopt;
}

// -------------------------------------------------------- //
//                    Policy Chain Set                      //
// -------------------------------------------------------- //

///
/// Constructs a new PolicyChainSet by iterating over all the PolicyChain(s),
/// and creating a list of them. All the underlying policy chains will be
/// defined as `::Selection`.
///
/// @param[in]  psets_defs  The list of policy chain's definitions
/// @param      lua         The lua state
///
PolicyChainSet::PolicyChainSet(
    const std::vector<std::vector<std::string>> &psets_defs, sol::state &lua) {
  for (const auto &pchain_def : psets_defs) {

    // All the policy chains will be declared as ::Selection
    PolicyChain pchain{pchain_def, PolicyChainType::Selection, lua};
    if (pchain.empty()) {
      continue;
    }
    pchains.emplace_back(pchain);
  }
}

///
/// It chronologically applies all the available policy chains on the experiment
/// and returns a list of submissions that are satisfies the **first**
/// PolicyChain in the list. If none of the chains were able to select any
/// submissions, an empty `std::optional` will be returned.
///
/// @param      expr  The experiment
///
/// @return     An optional list of submissions
///
std::optional<std::vector<Submission>>
PolicyChainSet::operator()(Experiment &expr) {

  for (auto &pchain : pchains) {

    auto selection = pchain(expr);

    // If any of the pchains return something, we ignore the rest, and leave!
    if (selection) {
      return selection;
    }
  }

  return std::nullopt;
}

///
/// It chronologically applies all the available policy chains on the submission
/// pool and returns a list of submissions that are satisfies the **first**
/// PolicyChain in the list. If none of the chains were able to select any
/// submissions, an empty `std::optional` will be returned.
///
/// @param      spool  The pool of submissions
///
/// @return     An optional list of submissions
///
std::optional<std::vector<Submission>>
PolicyChainSet::operator()(std::vector<Submission> &spool) {

  if (empty()) {
    return spool;
  }

  for (auto &pchain : pchains) {

    auto selection = pchain(spool);

    // If any of the pchains return something, we ignore the rest, and leave!
    if (selection) {
      return selection;
    }
  }

  return std::nullopt;
}
