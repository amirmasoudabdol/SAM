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
      res = this->perturb(experiment, params.num);
    else if (params.approach == "swapping groups")
      res = this->swapGroups(experiment, params.num);
    else if (params.approach == "switching groups")
      res = this->switchGroups(experiment, params.num);
    
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

bool FalsifyingData::perturb(Experiment *experiment, const int n) {
  
  static arma::Row<double> noise(n, arma::fill::zeros);
  
  int begin {0}, end {0};
  std::tie(begin, end) = getTargetBounds(experiment, params.target);
  
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
  
  std::cout << "TO BE IMPLEMENTED!";
  
  // Success Code
  return true;
}


bool FalsifyingData::switchGroups(Experiment *experiment, const int n) {
  
  std::cout << "TO BE IMPLEMENTED!";
  
  // Success Code
  return true;
}
