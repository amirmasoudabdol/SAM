//
// Created by Amir Masoud Abdol on 2020-08-28.
//

#include "HackingStrategy.h"

using namespace sam;

void FabricatingData::perform(Experiment *experiment) {
  
  spdlog::debug("Falsifying Data: ");
  
  bool res {true};
  
  for (int t = 0; t < params.n_attempts && res; ++t) {
    
    if (params.approach == "generating")
      res = this->generate(experiment, params.nums);
    else if (params.approach == "duplicating")
      res = this->duplicate(experiment, params.nums);
    
    experiment->recalculateEverything();
    
    if(!params.stopping_cond_defs.empty()) {
      if (stopping_condition(experiment)) {
        spdlog::trace("⚠️ Stopping the hacking procedure, stopping condition has been met!");
        return;
      }
    }
    
    spdlog::trace("{}", *experiment);
    
  }
  
}


bool FabricatingData::generate(Experiment *experiment, const int n) {
  
  int begin {0}, end {0};
  if (params.target == "control") {begin = 0; end = experiment->setup.nd();}
  else if (params.target == "treatment") {begin = experiment->setup.nd(); end = experiment->setup.ng();}
  else if (params.target == "both") {begin = 0; end = experiment->setup.ng();}
  
  auto new_observations =
  experiment->data_strategy->genNewObservationsForAllGroups(experiment, n);
  
  for (int i = begin; i < end; ++i) {
    
    experiment->groups_[i].add_measurements(new_observations[i]);
    
  }
  
  return true;
}


bool FabricatingData::duplicate(Experiment *experiment, const int n) {
  
  int begin {0}, end {0};
  if (params.target == "control") {begin = 0; end = experiment->setup.nd();}
  else if (params.target == "treatment") {begin = experiment->setup.nd(); end = experiment->setup.ng();}
  else if (params.target == "both") {begin = 0; end = experiment->setup.ng();}
  
  for (int i = begin; i < end; ++i) {
    
    arma::Row<double> row = arma::shuffle((*experiment)[i].measurements());
    arma::Row<double> copy_candidates = row.head(n);
    
    experiment->groups_[i].add_measurements(copy_candidates);
    
  }
  
  return true;
}
