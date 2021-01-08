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
      res = this->perturb(experiment);
    else if (params.approach == "swapping")
      res = this->swapGroups(experiment);
    else if (params.approach == "switching")
      res = this->switchGroups(experiment);
    
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

bool FalsifyingData::perturb(Experiment *experiment) {
  
  spdlog::debug(" → Perturbing some data points...");
  
  int begin {0}, end {0};
  std::tie(begin, end) = getTargetBounds(experiment, params.target);
  
  for (int i = begin; i < end; ++i) {
    
    auto &row = (*experiment)[i].measurements();
    
    // Making sure that there is enough elements to select
    size_t num = std::min(params.num, static_cast<size_t>(experiment->groups_[i].measurements().n_elem));
    
    // Selecting n indices randomly
    arma::uvec shuffled_indices = arma::shuffle(arma::regspace<arma::uvec>(0, 1, row.n_elem - 1));
    arma::uvec candidate_indices = shuffled_indices.head(num);
    
    static arma::Row<double> noise(num, arma::fill::zeros);
    noise.imbue([&](){
      return Random::get(params.noise_dist.value());
    });
    
    row.elem(candidate_indices) += noise;
    
  }
  
  // Success Code
  return true;
}


bool FalsifyingData::swapGroups(Experiment *experiment) {
  
  spdlog::debug(" → Swapping some data points...");
  
  /// @todo This is a rather expensive implementation, I need to see if I can find
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
    
    // Making sure that there is enough elements to select. There should be equal number in each group
    size_t num = std::min(params.num,
                          static_cast<size_t>(std::min(experiment->groups_[d].measurements().n_elem,
                                                       experiment->groups_[i].measurements().n_elem)));
        
    // Candidates from Control group
    arma::rowvec C_cand_values = experiment->groups_[d].measurements().head(num);
    experiment->groups_[d].del_measurements(arma::regspace<arma::uvec>(0, 1, num - 1));

    // Candidates from Treatment group
    arma::rowvec T_cand_values = experiment->groups_[i].measurements().head(num);
    experiment->groups_[i].del_measurements(arma::regspace<arma::uvec>(0, 1, num - 1));

    // --- Actual swapping
    experiment->groups_[d].add_measurements(T_cand_values);
    experiment->groups_[i].add_measurements(C_cand_values);
        
  }
  
  // Success Code
  return true;
}


bool FalsifyingData::switchGroups(Experiment *experiment) {
  
  spdlog::debug(" → Switching some data points...");
  
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {
    
    if (params.switching_direction == "control-to-treatment") {
      if (params.selection_method == "random") {
        // Shuffling the data because I don't know its status. Better safe than sorry!
        Random::shuffle(experiment->groups_[d].measurements());
      } else {
        experiment->groups_[d].measurements() = arma::sort(experiment->groups_[d].measurements(), "descend");
      }
      
      // Making sure that there is enough elements to select. Only concerned about one group
      size_t num = std::min(params.num, static_cast<size_t>(experiment->groups_[d].measurements().n_elem));

      arma::rowvec C_cand_values = experiment->groups_[d].measurements().head(num);
      experiment->groups_[d].del_measurements(arma::regspace<arma::uvec>(0, 1, num - 1));
      experiment->groups_[i].add_measurements(C_cand_values);
      
    } else {
      if (params.selection_method == "random") {
        // Shuffling the data because I don't know its status. Better safe than sorry!
        Random::shuffle(experiment->groups_[i].measurements());
      } else {
        experiment->groups_[i].measurements() = arma::sort(experiment->groups_[i].measurements(), "ascend");
      }
      
      // Making sure that there is enough elements to select. Only concerned about one group
      size_t num = std::min(params.num, static_cast<size_t>(experiment->groups_[i].measurements().n_elem));

      arma::rowvec T_cand_values = experiment->groups_[i].measurements().head(num);
      experiment->groups_[i].del_measurements(arma::regspace<arma::uvec>(0, 1, num - 1));
      experiment->groups_[d].add_measurements(T_cand_values);

    }
    
  }
  
  // Success Code
  return true;
}
