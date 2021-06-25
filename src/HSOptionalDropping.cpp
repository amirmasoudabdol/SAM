//
// Created by Amir Masoud Abdol on 25/06/2021.
//

#include "HackingStrategy.h"

using namespace sam;


void OptionalDropping::perform(Experiment *experiment) {
  spdlog::debug("Optional Dropping: ");
  
  
  if (experiment->hasCovariants()) {
    // This generates covariants only if it's not already been generated!
    experiment->generateCovariants();
  }
  
  for (auto &conds : params.pooled) {
    
    for (auto &by : params.split_by) {
      int ng = experiment->setup.ng();
      
      auto splitted_dvs = split(experiment, conds, by, ng);
      
      experiment->setup.setNC(experiment->setup.nc() + conds.size());
      experiment->dvs_.insert(experiment->dvs_.end(), splitted_dvs.begin(), splitted_dvs.end());
      
      spdlog::trace("{}", *experiment);
      
      experiment->recalculateEverything();
      
      spdlog::trace("{}", *experiment);
      
      if (!params.stopping_cond_defs.empty()) {
        if (stopping_condition(experiment)) {
          spdlog::trace("⚠️ Stopping the hacking procedure, stopping condition has been met!");
          return;
        }
      }
      
    }
  }
  
  spdlog::trace("{}", *experiment);
}


std::vector<DependentVariable> OptionalDropping::split(Experiment *experiment, std::vector<int>& conds, std::vector<int> &by, int ng) {
  
  std::vector<DependentVariable> splitted_dvs;
  std::vector<std::vector<int>> dv_pairs(experiment->setup.nd());
  
  // generating a table of indices for dvs that are going to be pooled together
  /// @todo This can be refactored, as it's been used in Group Pooling as well
  for (int i{0}; i < experiment->setup.nd(); ++i) {
    for (auto &c : conds) {
      dv_pairs[i].push_back(experiment->setup.nd() * c + i);
    }
  }
  
  /// Getting the right indices for splitting
  arma::uvec indices = arma::find(experiment->covariants.col(by[0]) == by[1]);
  
  /// actually pooling the dvs together
  for (auto &gs : dv_pairs) {
    
    for (auto &g : gs) {
      
      DependentVariable spl_dv;
      spl_dv.addNewMeasurements(experiment->dvs_[g].measurements().elem(indices).as_row());
      splitted_dvs.push_back(spl_dv);
      
      // updating the id; otherwise, it's Lua would have problem with filtering
      splitted_dvs.back().id_ = ng++;
      
    }
  }
  
  return splitted_dvs;
}
