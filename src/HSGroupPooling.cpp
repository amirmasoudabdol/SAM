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

/// Create a new group by pooling (adding) data from `r` groups together.
/// This literally appends the data of selected groups to each other to create a
/// new group.
///
/// @param experiment a pointer to the given experiment. Note: This can be a
/// copy of the experiment, based on the preferences of the researcher.
/// @param r Lenght of each permutation
void GroupPooling::pool(Experiment *experiment, int r) {
  // Length of each permutation
  //    const int r = _num;
  
  // Original number of conditions
  const int n = experiment->setup.nc();
  
  // List of groups participating in the pool.
  std::vector<int> v(n);
  
  /// Note: This starts from `1` because the first group is the control group
  /// and it should not be included in the pooling. See #194. This is a bit
  /// different than before where I was using population parameters to run the
  /// t-test.
  std::iota(v.begin(), v.end(), 1); // Filling v with range(1, n)
  
  // Gets the list of all permutation
  std::vector<std::vector<int>> permutations =
  for_each_reversible_circular_permutation(v.begin(), v.begin() + r,
                                           v.end(), collector());
  
  std::vector<arma::Row<float>> pooled_groups;
  
  // Extending the measurements to avoid over-pooling, see #104
  //    experiment->measurements.resize(experiment->setup.ng() +
  //    permutations.size() * experiment->setup.nd());
  
  //    for (auto &per : permutations) {
  //
  //        for (int d = 0; d < experiment->setup.nd(); d++) {
  //
  //            // Creating an empty new group
  ////            experiment->measurements.push_back(arma::Row<float>());
  //
  ////            arma::Row<float> new_group{experiment->measurements[per[0]]};
  //
  //            for (int i = 0; i < r ; i++){
  //
  ////                new_group += experiment->measurements[per[i] * (n - 1) +
  /// d];
  //                experiment->measurements[experiment->setup.ng() + d]
  //                                .insert_cols(experiment->measurements.back().size(),
  //                                              experiment->measurements[per[i]
  //                                              * (n - 1) + d]);
  //
  //            }
  ////            std::cout << new_group  << std::endl;
  //            // Fill the new group by pooling members of the selected
  //            permutation, `per`.
  ////            experiment->measurements[experiment->setup.ng() + d] =
  /// new_group;
  //
  //        }
  //
  //    }
}
