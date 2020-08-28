//
// Created by Amir Masoud Abdol on 2020-08-28.
//

#include "HackingStrategy.h"

using namespace sam;

void FalsifyingData::perform(Experiment *experiment) {
  
  spdlog::debug("Falsifying Data: ");
  
  bool res {true};
  
  for (int t = 0; t < params.n_attempts && res; ++t) {
    
    if (params.approach == "perturbation")
      res = this->perturb(experiment, params.nums);
    else
      res = this->swapGroups(experiment, params.nums);
    
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

bool FalsifyingData::perturb(Experiment *experiment, const int n) {
  
  static arma::Row<double> noise(n, arma::fill::zeros);
  
  int begin {0}, end {0};
  if (params.target == "control") {begin = 0; end = experiment->setup.nd();}
  else if (params.target == "treatment") {begin = experiment->setup.nd(); end = experiment->setup.ng();}
  else if (params.target == "both") {begin = 0; end = experiment->setup.ng();}
  
  for (int i = begin; i < end; ++i) {
    
    auto &row = (*experiment)[i].measurements();
    
    // Selecting n indices randomly
    arma::uvec shuffled_indices = arma::shuffle(arma::regspace<arma::uvec>(0, 1, row.n_elem - 1));
    arma::uvec candicate_indices = shuffled_indices.head(n);
    
    noise.imbue([&](){
      return Random::get(params.noise_dist.value());
    });
    
    row.elem(candicate_indices) += noise;
    
  }
  
  // Success Code
  return true;
}


bool FalsifyingData::swapGroups(Experiment *experiment, const int n) {
  
  
  // Success Code
  return true;
}
