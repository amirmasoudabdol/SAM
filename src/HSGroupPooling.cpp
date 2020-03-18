//
// Created by Amir Masoud Abdol on 18/03/2020.
//

#include "utils/permutation.h"

#include "HackingStrategy.h"

using namespace sam;

/**
 Perform Group Pooling on the given Experiment.

 @param experiment A pointer to researcher's experiment
 @param decisionStrategy A pointer to researcher's decision strategy
 */
void GroupPooling::perform(Experiment *experiment,
                           DecisionStrategy *decisionStrategy) {

  //    spdlog::debug("Group Pooling");
  //
  //    if (experiment->setup.nc() < 2){
  //        /// TODO: This should probably not be a throw and just a
  //        /// message. It's not a big deal after all, and I don't want to stop
  //        the program from running throw std::domain_error("There is not
  //        enough groups for pooling.");
  //    }
  //
  //    // Pooling groups together
  //    for (auto &r : params.nums){
  //        pool(experiment, r);
  //    }
  //
  //    // TODO: Improve me! This is very ugly and prune to error
  //    int new_ng = experiment->setup.ng();
  //
  //    std::logic_error("I'm broken! Don't use me yet!");
  //    experiment->initResources();
  //
  //    experiment->calculateStatistics();
  //    experiment->calculateEffects();
  //    experiment->runTest();
  //
  //    if (!decisionStrategy->verdict(*experiment,
  //    DecisionStage::WhileHacking).isStillHacking()){
  //        return ;
  //    }
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

  std::vector<arma::Row<double>> pooled_groups;

  // Extending the measurements to avoid over-pooling, see #104
  //    experiment->measurements.resize(experiment->setup.ng() +
  //    permutations.size() * experiment->setup.nd());

  //    for (auto &per : permutations) {
  //
  //        for (int d = 0; d < experiment->setup.nd(); d++) {
  //
  //            // Creating an empty new group
  ////            experiment->measurements.push_back(arma::Row<double>());
  //
  ////            arma::Row<double> new_group{experiment->measurements[per[0]]};
  //
  //            for (int i = 0; i < r ; i++){
  //
  ////                new_group += experiment->measurements[per[i] * (n - 1) +
  ///d];
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
  ///new_group;
  //
  //        }
  //
  //    }
}