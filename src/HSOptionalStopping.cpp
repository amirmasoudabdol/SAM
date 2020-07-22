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
  
  for (int t = 0; t < params.n_attempts && t < params.max_attempts; ++t) {
    spdlog::debug("\t #{} attempt(s)", t);
    
    /// \todo this can be improved, I think I should send an array to addObservations for
    /// each group, maybe an overload of it with this condition
    if (params.num == 0) {
      n_new_items = static_cast<int>(params.add_by_fraction * experiment->groups_[0].nobs_);
    }
    addObservations(experiment, n_new_items);

    /// \todo: This can still be done nicer
    experiment->calculateStatistics();
    experiment->calculateEffects();
    experiment->runTest();

    /// \todo Implement a stopping condition if it makes sense
    
    
    
  }
  
  for (int g{0}; g < experiment->setup.ng(); ++g) {
    spdlog::debug("\t {}", (*experiment)[g]);
  }
}

/// \todo This can be static and be used by other similar methods, e.g., Subjective Optional Stopping
void OptionalStopping::addObservations(Experiment *experiment, const int n) {

  // Get the new observations
  auto new_observations =
      experiment->data_strategy->genNewObservationsForAllGroups(experiment, n);
  
  for (int g{0}; g < experiment->setup.ng(); ++g) {
    (*experiment)[g].add_measurements(new_observations[g]);
  }
  
}
