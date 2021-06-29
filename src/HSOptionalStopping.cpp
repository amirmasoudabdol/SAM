//===-- HSOptionalStopping.cpp - Optional Stopping Implementation ---------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 18/03/2020.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the implementation of the Optional Stopping hacking
/// strategy class, and all their dependent routines.
///
//===----------------------------------------------------------------------===//

#include "HackingStrategy.h"

using namespace sam;

/// Implementation of Optional Stopping method
///
/// Iterates for maximum `n_attempts` and add new observations to experiment's
/// groups. The number of new observations to be added to each group is
/// either a fixed number indicated by `params.num` or a variable number for
/// each group, calculated as a fraction of already existing observations,
/// `params.ratio`.
///
/// After each optional stopping step, all statistics will be recalculated and
/// experiment will be passed to the `hackTheResearch` for further
/// Selection → Decision sequence.
void OptionalStopping::perform(Experiment *experiment) {

  spdlog::debug("Optional Stopping: ");

  // Determining the number of new observations to be added to each group
  arma::Row<int> ns(experiment->setup.ng());
  if (params.num.is_empty()) {

    // Getting one random number since I want each group to have the same
    // number of observations
    float fraction{params.ratio};

    // Filling up a vector of n_new_obs for each group
    ns.imbue([&, i = 0]() mutable {
      return static_cast<int>(std::floor(
          fraction * static_cast<float>(experiment->dvs_[i++].nobs_)));
    });
  } else {
    ns.fill(static_cast<int>(params.num));
  }

  // n_attempts is a Parameter<int> therefore, I need to ask for a value instead
  // of using it directly
  int n_at{params.n_attempts};

  for (int t = 0; t < n_at; ++t) {
    spdlog::trace("\t #{} attempt(s)", t + 1);

    addObservations(experiment, ns);

    experiment->recalculateEverything();

    if (!params.stopping_cond_defs.empty()) {
      if (stopping_condition(experiment)) {
        spdlog::trace(
            "⚠️ Stopping the hacking procedure, stopping condition "
            "has been met!");
        return;
      }
    }
  }

  spdlog::trace("{}", *experiment);
}

///
/// @brief      Adds new observations to every group
///
/// @param      experiment  The pointer to the Experiment
/// @param[in]  ns          Indicates the number of new observations to be added
///                         to each group.
///
void OptionalStopping::addObservations(Experiment *experiment,
                                       const arma::Row<int> &ns) {

  // Getting max(ns) observations. Sending max to the method is necessary due to
  // the possibility of dealing with multivariate distribution. After getting
  // enough observations, I will only add what I need based on ns[i] to each
  // group
  auto new_observations =
      experiment->data_strategy->genNewObservationsForAllGroups(experiment,
                                                                ns.max());

  int begin{0};
  int end{0};
  std::tie(begin, end) = getTargetBounds(experiment, params.target);

  // Distributing new items according to the requested size
  for (int i = begin; i < end; ++i) {
    (*experiment)[i].addNewMeasurements(new_observations[i].head(ns.at(i)));
  }
}
