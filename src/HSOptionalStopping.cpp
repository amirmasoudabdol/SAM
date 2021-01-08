//
// Created by Amir Masoud Abdol on 18/03/2020.
//

#include "HackingStrategy.h"

using namespace sam;

/// Implementation of Optional Stopping method
///
/// Iterates for maximum `n_attempts` and add new obversations to experiment's
/// groups. The number of new observations to be added to each group is deducted
/// either a fixed number indicated by `params.num` or a variable number for each
/// group, calculated as a fraction of already existing observations, `params.ratio`.
///
/// After each optional stopping step, all statistics will be recalculated and experiment will
/// be passed to the `research` method.
void OptionalStopping::perform(Experiment *experiment) {

  spdlog::debug("Optional Stopping: ");

  
  /// Determining the number of new observations to be added to each group
  arma::Row<int> ns(experiment->setup.ng());
  if (params.num.is_empty()) {
    
    /// Getting one random number since I want each group to have the same number of observations
    double fraction {params.ratio};
    
    ns.imbue([&, i = 0]() mutable {
      return std::floor(fraction * experiment->groups_[i++].nobs_);
    });
  }else{
    ns.fill(static_cast<int>(params.num));
  }
  
  int n_at {params.n_attempts};
  for (int t = 0; t < n_at; ++t) {
    spdlog::trace("\t #{} attempt(s)", t + 1);
    
    addObservations(experiment, ns);
    
    experiment->recalculateEverything();

    if (!params.stopping_cond_defs.empty()) {
      if (stopping_condition(experiment)) {
        spdlog::trace("⚠️ Stopping the hacking procedure, stopping condition has been met!");
        return;
      }
    }

  }
  
  spdlog::trace("{}", *experiment);

}

void OptionalStopping::addObservations(Experiment *experiment, const int n) {

  // Get the new observations
  auto new_observations =
      experiment->data_strategy->genNewObservationsForAllGroups(experiment, n);
  
  
  int begin {0}, end {0};
  std::tie(begin, end) = getTargetBounds(experiment, params.target);
  
  for (int i = begin; i < end; ++i) {
    (*experiment)[i].add_measurements(new_observations[i]);
  }
  
}

void OptionalStopping::addObservations(Experiment *experiment, const arma::Row<int> ns) {
  
  // Getting max(ns) observations, sending max to the method is necessary due to the possibility
  // of dealing with multivariate distribution
  auto new_observations = experiment->data_strategy->genNewObservationsForAllGroups(experiment,
                                                                                    ns.max());
  
  int begin {0}, end {0};
  std::tie(begin, end) = getTargetBounds(experiment, params.target);
  
  // Distributing new items according to the requested size
  for (int i = begin; i < end; ++i) {
    (*experiment)[i].add_measurements(new_observations[i].head(ns.at(i)));
  }
  
}
