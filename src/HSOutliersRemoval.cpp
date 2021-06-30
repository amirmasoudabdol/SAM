//===-- HSOutliersRemoval.cpp - Outliers Removal Implementation -----------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 18/03/2020.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the implementation of the Outliers Removal hacking
/// strategy class, and all its dependencies.
///
//===----------------------------------------------------------------------===//

#include "HackingStrategy.h"

using namespace sam;

/// @brief      Implementation of the Outliers Removal hacking strategy
///
/// For each multiplier, the algorithm iterates for maximum of `n_attempts` and
/// removes `num` observations from the experiment based on the defined
/// parameters. At any points during the execution if the stopping_condition
/// met, the outliers removal will be stopped, and the last alteration of the
/// experiment will be returned to the Researcher.
void OutliersRemoval::perform(Experiment *experiment) {

  spdlog::debug("Outliers Removal: ");

  for (const auto k : params.multipliers) {

    bool res{true};

    /// Removing outliers `n` at a time, for the total of `n_attempts`
    /// It'll stop either when n_attempts are exhausted, or there is no
    /// more observations left to be removed
    for (int t = 0; t < params.n_attempts && res; ++t) {

      res = this->removeOutliers(experiment, params.num, k, params.side);

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
}

///
/// At each iteration, the algorithm removes `n` observations that are further
/// their standardized distance from the mean is greater than `k`. The `side`
/// parameter can be used to indicate the side where outliers should be removed.
///
/// @param      experiment  The experiment
/// @param[in]  n           Indicates the number of outliers to be removed
/// @param[in]  k           Indicates the distance of outliers from the mean
/// @param[in]  side        Indicates the side of the mean where outliers should
/// be selected, and removed from the dataset.
///
/// @return     Returns `false` if the number of observations is less than the
/// `min_observations` dedicated by the user. This is a signal to `perform` to
/// stop its execution.
///
bool OutliersRemoval::removeOutliers(Experiment *experiment, const int n,
                                     const float k, const int side) {

  // Getting the boundary of the targeted group
  int begin{0};
  int end{0};
  std::tie(begin, end) = getTargetBounds(experiment, params.target);

  // Removing the outliers from the targeted groups
  for (int i = begin; i < end; ++i) {

    auto &row = (*experiment)[i].measurements();

    // At least one row has less than `min_observations`
    if (row.size() <= params.min_observations) {
      return false; // Unsuccessful return, nothing can be removed!
    }

    // This trick makes finding the largest outlier easier. I'll see if I can
    // find a better way
    if (params.order == "max first") {
      row = sort(row);
    }

    // Finding the outliers, returning only `n` of them
    arma::uvec inx;
    if (side == 0) {
      // Both
      inx = arma::find(arma::abs(row - (*experiment)[i].mean_) /
                               (*experiment)[i].stddev_ >
                           k,
                       n, "first");
    } else if (side > 0) {
      // Right
      inx = arma::find(
          (row - (*experiment)[i].mean_) / (*experiment)[i].stddev_ > k, n,
          "first");
    } else {
      // Left
      inx = arma::find(
          (row - (*experiment)[i].mean_) / (*experiment)[i].stddev_ < -1 * k, n,
          "first");
    }

    // If removing all the detected outliers lead to selected group having less
    // observations than `min_observations`, then the algorithm removes the
    // portion of the detected outliers
    if ((row.n_elem - inx.n_elem) <= params.min_observations) {
      inx = inx.head(row.n_elem - params.min_observations);
    }

    // actually removing them from the group
    (*experiment)[i].removeMeasurements(inx);
  }

  // Success Code
  return true;
}
