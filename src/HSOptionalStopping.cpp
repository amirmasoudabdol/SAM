//
// Created by Amir Masoud Abdol on 18/03/2020.
//

#include "HackingStrategy.h"

using namespace sam;

///
/// \brief      Implementation of optional stopping.
///
/// This will use two parameters set at construction of the OptionalStopping
/// class, `n_trials` and `n_new_obs` for every trial, the routine will add
/// `n_new_obs` to all groups, recalculate the statistics, and run the test. It
/// will then select an outcome based on researcher's preference and check it's
/// significance. If the results is significant, it'll not make a new attempt to
/// add more data, and will return to the hack() routine.
///
void OptionalStopping::perform(Experiment *experiment) {

  spdlog::debug("Optional Stopping: ");

  double n_new_items = params.num;
  
  for (int t = 0; t < params.n_attempts; ++t) {
    spdlog::debug("\t #{} attempt(s)", t);
    
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

    /// \todo Implement a stopping condition if it makes sense
    
    
    
  }
  
  for (int g{0}; g < experiment->setup.ng(); ++g) {
    spdlog::debug("\t {}", (*experiment)[g]);
  }
}

void OptionalStopping::addObservations(Experiment *experiment, const int n) {

  // Get the new observations
  auto new_observations =
      experiment->data_strategy->genNewObservationsForAllGroups(experiment, n);
  
  for (int g{0}; g < experiment->setup.ng(); ++g) {
    (*experiment)[g].add_measurements(new_observations[g]);
  }
  
}

void OptionalStopping::addObservations(Experiment *experiment, const arma::Row<int> ns) {
  
  // Getting max(ns) observations, sending max to the method is necessary due to the possibility
  // of dealing with multivariate distribution
  auto new_observations = experiment->data_strategy->genNewObservationsForAllGroups(experiment,
                                                                                    ns.max());
  
  // Distributing new items according to the requested size
  for (int g{0}; g < experiment->setup.ng(); ++g) {
    (*experiment)[g].add_measurements(new_observations[g].head(ns.at(g)));
  }
  
}
