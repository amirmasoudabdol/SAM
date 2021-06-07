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
      res = this->generate(experiment, params.num);
    else if (params.approach == "duplicating")
      res = this->duplicate(experiment, params.num);
    
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

bool FabricatingData::generate(Experiment *experiment, const int n) {
  
  int begin {0}, end {0};
  std::tie(begin, end) = getTargetBounds(experiment, params.target);
  
  auto new_observations =
  experiment->data_strategy->genNewObservationsForAllGroups(experiment, n);
  
  for (int i = begin; i < end; ++i) {
    
    experiment->dvs_[i].addNewMeasurements(new_observations[i]);
    
  }
  
  return true;
}


bool FabricatingData::duplicate(Experiment *experiment, const int n) {
  
  int begin {0}, end {0};
  std::tie(begin, end) = getTargetBounds(experiment, params.target);
  
  for (int i = begin; i < end; ++i) {
    
    arma::Row<float> row = arma::shuffle((*experiment)[i].measurements());
    arma::Row<float> copy_candidates = row.head(n);
    
    experiment->dvs_[i].addNewMeasurements(copy_candidates);
    
  }
  
  return true;
}
