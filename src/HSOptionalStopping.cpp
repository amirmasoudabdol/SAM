//
// Created by Amir Masoud Abdol on 18/03/2020.
//

#include "HackingStrategy.h"

using namespace sam;

/// Implementation of Optional Stopping method
///
/// Iterates for maximum `n_attemps` and add new obversations to experiment's
/// gourps. The number of new observations to be added to each group is deducted
/// either a fixed number indicated by `params.num` or a variable number for each
/// group, calculated as a fraction of already existing observations, `params.add_by_fraction`.
///
/// After each optional stopping step, all statistics will be recalculated and experiment will
/// be passed to the `research` method.
void OptionalStopping::perform(Experiment *experiment) {

  spdlog::debug("Optional Stopping: ");

  double n_new_items = params.num;
  
  for (int t = 0; t < params.n_attempts; ++t) {
    spdlog::debug("\t #{} attempt(s)", t+1);
    
    if (params.num == 0) {
      arma::Row<int> ns(experiment->setup.ng());
      ns.imbue([&, i = 0]() mutable {
        return params.add_by_fraction * experiment->groups_[i++].nobs_;
      });
      addObservations(experiment, ns);
    }else{
      addObservations(experiment, n_new_items);
    }
    
    experiment->recalculateEverything();

    if (!params.stopping_cond_defs.empty()) {
      if (stopping_condition(experiment)) {
        spdlog::trace("⚠️ Stopping the hacking procedure, stopping condition has been met!");
        return;
      }
    }

  }
  
  for (int g{0}; g < experiment->setup.ng(); ++g) {
    spdlog::debug("\t {}", (*experiment)[g]);
  }
}

void OptionalStopping::addObservations(Experiment *experiment, const int n) {

  // Get the new observations
  auto new_observations =
      experiment->data_strategy->genNewObservationsForAllGroups(experiment, n);
  
  
  int begin {0}, end {0};
  if (params.target == "control") {
    begin = 0; end = experiment->setup.nd();
  } else if (params.target == "treatment") {
    begin = experiment->setup.nd(); end = experiment->setup.ng();
  } else if (params.target == "both") {
    begin = 0; end = experiment->setup.ng();
  }
  
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
  if (params.target == "control") {
    begin = 0; end = experiment->setup.nd();
  } else if (params.target == "treatment") {
    begin = experiment->setup.nd(); end = experiment->setup.ng();
  } else if (params.target == "both") {
    begin = 0; end = experiment->setup.ng();
  }
  
  // Distributing new items according to the requested size
  for (int i = begin; i < end; ++i) {
    (*experiment)[i].add_measurements(new_observations[i].head(ns.at(i)));
  }
  
}
