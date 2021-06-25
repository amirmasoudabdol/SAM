//
// Created by Amir Masoud Abdol on 18/03/2020.
//

#include "DependentVariable.h"
#include "utils/permutation.h"

#include "HackingStrategy.h"
#include <algorithm>
#include <vector>

using namespace sam;

///
/// Perform Group Pooling on the given Experiment.
///
/// @param      experiment        A pointer to researcher's experiment
/// @param      researchStrategy  A pointer to researcher's research strategy
///
void GroupPooling::perform(Experiment *experiment) {
  
  spdlog::debug("Group Pooling");
  
  // Pooling groups together
  
  // we add a new condition for every new pooled group
  for (auto &conds : params.pooled_conditions){
    
    
    // keeping the original ng_ to be able to calculate the iD of the new dvs_
    int ng = experiment->setup.ng();
    
    /// @todo this can be replaced by addNewDependentVariable
    auto pooled_dv = pool(experiment, conds, ng);
    
    /// adding a new group
    /// @todo This can be replaced by addNewCondition
    experiment->setup.setNC(experiment->setup.nc() + 1);
    experiment->dvs_.insert(experiment->dvs_.end(), pooled_dv.begin(), pooled_dv.end());
    
  }
  
  experiment->recalculateEverything();
  
  spdlog::trace("{}", *experiment);
}

std::vector<DependentVariable> GroupPooling::pool(Experiment *experiment, std::vector<int>& conds, int ng) {
  
  std::vector<DependentVariable> pooled_dvs;
  std::vector<std::vector<int>> gs(experiment->setup.nd());
  
  /// @todo this can be replaced by std::algorithm
  
  // generating a table of indices for dvs that are going to be pooled together
  for (int i{0}; i < experiment->setup.nd(); ++i) {
    for (auto &c : conds) {
      gs[i].push_back(experiment->setup.nd() * c + i);
    }
  }
  
  /// actually pooling the dvs together
  for (auto &g : gs) {
    pooled_dvs.push_back(pool(experiment, g));
    
    // updating the id; otherwise, it's Lua would have problem with filtering
    pooled_dvs.back().id_ = ng++;
  }
  
  return pooled_dvs;
  
}

DependentVariable GroupPooling::pool(Experiment *experiment, std::vector<int>& gs) {
  
  DependentVariable grouped_dv;
  for (auto &g : gs) {
    grouped_dv.addNewMeasurements(experiment->dvs_[g].measurements());
  }
  
  return grouped_dv;
  
}
