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


  // actually splitting the groups
  spdlog::trace("Filtering by: [{}]", fmt::join(params.split_by.begin(), params.split_by.end(), ", "));
  split(experiment, params.split_by);

  experiment->recalculateEverything();

  spdlog::trace("{}", *experiment);

  if (!params.stopping_cond_defs.empty()) {
    if (stopping_condition(experiment)) {
      spdlog::trace("⚠️ Stopping the hacking procedure, stopping "
                    "condition has been met!");
      return;
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
void OptionalDropping::split(Experiment *experiment,
                               std::vector<int> &by) {

  /// Getting the right indices for splitting
  arma::uvec indices = arma::find(experiment->covariants.col(by[0]) == by[1]);

  /// actually pooling the dvs together

  for (auto &dv : experiment->dvs_) {
      dv.removeMeasurements(indices);
  }

}
