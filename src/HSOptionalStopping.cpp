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

  for (int t = 0; t < params.n_attempts && t < params.max_attempts; ++t) {

    addObservations(experiment, params.num);

    /// \todo: This can still be done nicer
    experiment->calculateStatistics();
    experiment->calculateEffects();
    experiment->runTest();

    /// \todo Implement a stopping condition if it makes sense
    
  }
  
  for (auto &g : experiment->groups_)
    spdlog::debug("\t{}", g);
}

void OptionalStopping::addObservations(Experiment *experiment, const int n) {

  // Get the new observations
  auto new_observations =
      experiment->data_strategy->genNewObservationsForAllGroups(experiment, n);
  
  for (int g{0}; g < experiment->setup.ng(); ++g) {
    (*experiment)[g].add_measurements(new_observations[g]);
  }
  
}
