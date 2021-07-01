//
// Created by Amir Masoud Abdol on 2020-08-28.
//

#include "HackingStrategy.h"

using namespace sam;

void FalsifyingData::perform(Experiment *experiment) {
  
  spdlog::debug("Falsifying Data: ");
  
  bool res {true};
  
  for (int t = 0; t < params.n_attempts && res; ++t) {
    
    if (params.approach == "perturbation") {
      res = this->perturb(experiment);
    } else if (params.approach == "swapping") {
      res = this->swapGroups(experiment);
    } else if (params.approach == "switching") {
      res = this->switchGroups(experiment);
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

bool FalsifyingData::perturb(Experiment *experiment) {
  
  spdlog::debug(" → Perturbing some data points...");
  
  int begin {0};
  int end {0};
  std::tie(begin, end) = getTargetBounds(experiment, params.target);
  
  for (int i = begin; i < end; ++i) {
    
    auto &row = (*experiment)[i].measurements();
    
    // Making sure that there is enough elements to select
    size_t num = std::min(params.num, static_cast<size_t>(experiment->dvs_[i].measurements().n_elem));
    
    // Selecting n indices randomly
    arma::uvec shuffled_indices = arma::shuffle(arma::regspace<arma::uvec>(0, 1, row.n_elem - 1));
    arma::uvec candidate_indices = shuffled_indices.head(num);
    
    static arma::Row<float> noise(num, arma::fill::zeros);
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
  /// @note Maybe I can actually implement something in the DependentVariable to do this nicer
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {
    
    if (params.selection_method == "random") {
      // Shuffling the data because I don't know its status. Better safe than sorry!
      Random::shuffle(experiment->dvs_[d].measurements());
      Random::shuffle(experiment->dvs_[i].measurements());
    } else {
      experiment->dvs_[d].measurements() = arma::sort(experiment->dvs_[d].measurements(), "descend");
      experiment->dvs_[i].measurements() = arma::sort(experiment->dvs_[i].measurements(), "ascend");
    }
    
    // Making sure that there is enough elements to select. There should be equal number in each group
    size_t num = std::min(params.num,
                          static_cast<size_t>(std::min(experiment->dvs_[d].measurements().n_elem,
                                                       experiment->dvs_[i].measurements().n_elem)));
        
    // Candidates from Control group
    arma::Row<float> C_cand_values = experiment->dvs_[d].measurements().head(num);
    experiment->dvs_[d].removeMeasurements(arma::regspace<arma::uvec>(0, 1, num - 1));

    // Candidates from Treatment group
    arma::Row<float> T_cand_values = experiment->dvs_[i].measurements().head(num);
    experiment->dvs_[i].removeMeasurements(arma::regspace<arma::uvec>(0, 1, num - 1));

    // --- Actual swapping
    experiment->dvs_[d].addNewMeasurements(T_cand_values);
    experiment->dvs_[i].addNewMeasurements(C_cand_values);
        
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
        Random::shuffle(experiment->dvs_[d].measurements());
      } else {
        experiment->dvs_[d].measurements() = arma::sort(experiment->dvs_[d].measurements(), "descend");
      }
      
      // Making sure that there is enough elements to select. Only concerned about one group
      size_t num = std::min(params.num, static_cast<size_t>(experiment->dvs_[d].measurements().n_elem));

      arma::Row<float> C_cand_values = experiment->dvs_[d].measurements().head(num);
      experiment->dvs_[d].removeMeasurements(arma::regspace<arma::uvec>(0, 1, num - 1));
      experiment->dvs_[i].addNewMeasurements(C_cand_values);
      
    } else {
      if (params.selection_method == "random") {
        // Shuffling the data because I don't know its status. Better safe than sorry!
        Random::shuffle(experiment->dvs_[i].measurements());
      } else {
        experiment->dvs_[i].measurements() = arma::sort(experiment->dvs_[i].measurements(), "ascend");
      }
      
      // Making sure that there is enough elements to select. Only concerned about one group
      size_t num = std::min(params.num, static_cast<size_t>(experiment->dvs_[i].measurements().n_elem));

      arma::Row<float> T_cand_values = experiment->dvs_[i].measurements().head(num);
      experiment->dvs_[i].removeMeasurements(arma::regspace<arma::uvec>(0, 1, num - 1));
      experiment->dvs_[d].addNewMeasurements(T_cand_values);

    }
    
  }
  
  // Success Code
  return true;
}