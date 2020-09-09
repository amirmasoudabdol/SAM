//
// Created by Amir Masoud Abdol on 2020-09-08.
//

#include "HackingStrategy.h"

using namespace sam;

void StoppingDataCollection::perform(Experiment *experiment) {
  
  spdlog::debug("Stopping Data Collection: ");
  
  auto groups = experiment->groups_;
  
  int n_obs_max = experiment->setup.nobs().max();
  int n_trials = n_obs_max / params.batch_size;
  
  spdlog::trace("In {} steps...", n_trials);
  
  /// @todo Check whether this overflow. As I'm using the n_obs_max, there might
  /// be cases that I don't really get enough observations to add, or I add less, or more
  /// to a group
  for (size_t t {0}; t < n_trials; ++t) {
    
    spdlog::debug("Adding {} new items.", (t + 1) * params.batch_size);
    for (int g{0}; g < experiment->setup.ng(); ++g) {

      (*experiment)[g].set_measurements(groups[g].measurements().head((t + 1) * params.batch_size));
    }
    
    experiment->recalculateEverything();
    
    spdlog::trace("{}", *experiment);
    
    if(!params.stopping_cond_defs.empty()) {
      if (stopping_condition(experiment)) {
        spdlog::trace("⚠️ Stopping the hacking procedure, stopping condition has been met!");
        return;
      }
    }
    
  }
  
}

