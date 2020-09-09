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
    else if (params.approach == "swapping")
      res = this->swapGroups(experiment, params.num);
    else if (params.approach == "switching")
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
  
  spdlog::debug("\t Perturbing some data points...");
  
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
  
  spdlog::debug("\t Swapping some data points...");
  
  /// @todo This is a rahter expensive implementation, I need to see if I can find
  /// something in STL to do it better
  /// @note Maybe I can actually implement something in the Group to do this nicer
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {
    
    if (params.selection_method == "random") {
      // Shuffling the data because I don't know its status. Better safe than sorry!
      Random::shuffle(experiment->groups_[d].measurements());
      Random::shuffle(experiment->groups_[i].measurements());
    } else {
      experiment->groups_[d].measurements() = arma::sort(experiment->groups_[d].measurements(), "descend");
      experiment->groups_[i].measurements() = arma::sort(experiment->groups_[i].measurements(), "ascend");
    }
        
    // Candidates from Control group
    arma::rowvec C_cand_values = experiment->groups_[d].measurements().head(params.num);
    experiment->groups_[d].del_measurements(arma::regspace<arma::uvec>(0, 1, params.num - 1));

    // Candidates from Treatment group
    arma::rowvec T_cand_values = experiment->groups_[i].measurements().head(params.num);
    experiment->groups_[i].del_measurements(arma::regspace<arma::uvec>(0, 1, params.num - 1));

    // --- Actual swapping
    experiment->groups_[d].add_measurements(T_cand_values);
    experiment->groups_[i].add_measurements(C_cand_values);
        
  }
  
  // Success Code
  return true;
}


bool FalsifyingData::switchGroups(Experiment *experiment, const int n) {
  
  spdlog::debug("\t Switching some data points...");
  
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {
    
    if (params.switching_direction == "control-to-treatment") {
      if (params.selection_method == "random") {
        // Shuffling the data because I don't know its status. Better safe than sorry!
        Random::shuffle(experiment->groups_[d].measurements());
      } else {
        experiment->groups_[d].measurements() = arma::sort(experiment->groups_[d].measurements(), "descend");
      }

      arma::rowvec C_cand_values = experiment->groups_[d].measurements().head(params.num);
      experiment->groups_[d].del_measurements(arma::regspace<arma::uvec>(0, 1, params.num - 1));
      experiment->groups_[i].add_measurements(C_cand_values);
      
    } else {
      if (params.selection_method == "random") {
        // Shuffling the data because I don't know its status. Better safe than sorry!
        Random::shuffle(experiment->groups_[i].measurements());
      } else {
        experiment->groups_[i].measurements() = arma::sort(experiment->groups_[i].measurements(), "ascend");
      }

      arma::rowvec T_cand_values = experiment->groups_[i].measurements().head(params.num);
      experiment->groups_[i].del_measurements(arma::regspace<arma::uvec>(0, 1, params.num - 1));
      experiment->groups_[d].add_measurements(T_cand_values);

    }
    
  }
  
  // Success Code
  return true;
}
