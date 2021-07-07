//===-- HSOptionalDropping.cpp - Optional Dropping Implementation ---------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 25/06/2021.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the implementation details of the Optional Dropping
/// hacking strategy and its related subroutines.
///
//===----------------------------------------------------------------------===//

#include "HackingStrategy.h"

using namespace sam;

///
/// @brief      Performs the Optional Dropping
///
/// The algorithm first generates a matrix of covariant variables based on
/// users' input from the Experiment. Then, it iterates over the selected pooled
/// of groups **and** pre-defined covariates and their values. In each
/// iteration,
///  members of each group will be filtered based on the selected value of the
///  covariates and a new group will be formed based on those selection.
///  Finally, the algorithm updates the Experiment, runs the statistical tests,
///  and checks the stopping_condition. This process continues until the
///  exhaustion of all combinations or passing of the stopping condition.
///
///
/// @param      experiment  The experiment
///
void OptionalDropping::perform(Experiment *experiment) {
  spdlog::debug("Optional Dropping: ");

  // This generates covariant only if it's not already been generated!
  if (experiment->hasCovariants()) {
    experiment->generateCovariants();
  } else {
    spdlog::critical("Covariant variables are not defined! Use `n_covariants` "
                     "to define the number of covariants.");
    exit(1);
  }

  // loop over a set of groups/conditions
  for (auto &conds : params.pooled) {

    // loop over the covariats and its selected levels,
    // e.g., [0, 1], splits the data wherever the first covariats is 1.
    for (auto &by : params.split_by) {

      // actually splitting the groups
      int ng = experiment->setup.ng();
      auto splitted_dvs = split(experiment, conds, by, ng);

      // updating the size of the experiment, and adding the new dvs to the back
      // of it
      experiment->setup.setNC(experiment->setup.nc() + conds.size());
      experiment->dvs_.insert(experiment->dvs_.end(), splitted_dvs.begin(),
                              splitted_dvs.end());

      experiment->recalculateEverything();

      spdlog::trace("{}", *experiment);

      if (!params.stopping_cond_defs.empty()) {
        if (stopping_condition(experiment)) {
          spdlog::trace("⚠️ Stopping the hacking procedure, stopping "
                        "condition has been met!");
          return;
        }
      }
    }
  }

  spdlog::trace("{}", *experiment);
}

///
/// @brief      Splits the given groups/conditions into a new group based on a
/// specific covariats and its value
///
/// @param      experiment  The experiment
/// @param      conds       Indicates a list of groups/conditions
/// @param      by          Indicates the index of a covariats and its value,
/// e.g., [0, 1]
/// @param[in]  ng          Indicates the index of last dependent variable in
/// the experiment before adding the new groups
///
/// @return     A list of new dependent variables
///
std::vector<DependentVariable> OptionalDropping::split(Experiment *experiment,
                                                       std::vector<int> &conds,
                                                       std::vector<int> &by,
                                                       int ng) {

  std::vector<DependentVariable> splitted_dvs;
  std::vector<std::vector<int>> dv_pairs(experiment->setup.nd());

  // generating a table of indices for dvs that are going to be pooled together
  /// @todo This can be refactored, as it's been used in Group Pooling as well
  for (int i{0}; i < experiment->setup.nd(); ++i) {
    for (auto &c : conds) {
      dv_pairs[i].push_back(experiment->setup.nd() * c + i);
    }
  }

  /// Getting the right indices for splitting
  arma::uvec indices = arma::find(experiment->covariants.col(by[0]) == by[1]);

  /// actually pooling the dvs together
  for (auto &gs : dv_pairs) {

    for (auto &g : gs) {

      DependentVariable spl_dv;
      spl_dv.addNewMeasurements(
          experiment->dvs_[g].measurements().elem(indices).as_row());
      splitted_dvs.push_back(spl_dv);

      // updating the id; otherwise, it's Lua would have problem with filtering
      splitted_dvs.back().id_ = ng++;
    }
  }

  return splitted_dvs;
}
