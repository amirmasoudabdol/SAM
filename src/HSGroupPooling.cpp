//
// Created by Amir Masoud Abdol on 18/03/2020.
//

#include "DependentVariable.h"

#include "HackingStrategy.h"
#include <algorithm>
#include <vector>

using namespace sam;

///
/// Perform Group Pooling on the given Experiment. By looping through selected
/// set of conditions, and pooling their dependent variables together.
///
/// @param      experiment        A pointer to researcher's experiment
/// @param      researchStrategy  A pointer to researcher's research strategy
///
void GroupPooling::perform(Experiment *experiment) {

  spdlog::debug("Group Pooling");

  // Checking if more than two groups are available.

  // Pooling groups together
  // we add a new condition for every new pooled group
  for (auto &conds_inx : params.pooled_conditions) {

    // Checking to see whether the pooling is possible
    if (std::any_of(conds_inx.begin(), conds_inx.end(), [&](auto &c_id) {
          return c_id > experiment->setup.nc();
        })) {
      spdlog::critical("Group pooling cannot be performed! The given group ids \
        do not match the number of available conditions.");
      spdlog::critical("Make sure that the value of `pooled_conditions` is \
        within the boundary of total number of conditions defined.");
    }

    // keeping the original ng_ to be able to calculate the iD of the new dvs_
    int ng = experiment->setup.ng();

    /// @todo @improvement this can be replaced by addNewDependentVariable
    auto pooled_dv = pool(experiment, conds_inx, ng);

    /// adding a new group
    /// @todo @improvement This can be replaced by addNewCondition
    experiment->setup.setNC(experiment->setup.nc() + 1);
    experiment->dvs_.insert(experiment->dvs_.end(), pooled_dv.begin(),
                            pooled_dv.end());

    experiment->recalculateEverything();

    // Stops the pooling as soon as the stopping condition has meet
    if (!params.stopping_cond_defs.empty()) {
      if (stopping_condition(experiment)) {
        spdlog::trace(
            "⚠️ Stopping the hacking procedure, stopping condition "
            "has been met!");
        return;
      }
    }
  }

  spdlog::trace("{}", *experiment);
}

///
/// Goes through every pair of conditions, pools all their dependent
/// variables together, and finally adds them to the end of the experiment.
///
/// @param      experiment  The experiment
/// @param      conds_inx   The indices of selected conditions
/// @param[in]  ng          The index of last DV before the actual pooling
///                         happens. This is being used to determine the id of
///                         each new DV.
///
/// @return     Returns a list of new dependent variables
///
std::vector<DependentVariable>
GroupPooling::pool(Experiment *experiment, std::vector<int> &conds_inx, int ng) {

  std::vector<DependentVariable> pooled_dvs;
  std::vector<std::vector<int>> dvs_inx(experiment->setup.nd());

  // generating a table of indices for dvs that are going to be pooled together
  // @todo @improvement this may be replaced by an std::algorithm
  for (int i{0}; i < experiment->setup.nd(); ++i) {
    for (auto &c : conds_inx) {
      dvs_inx[i].push_back(experiment->setup.nd() * c + i);
    }
  }

  // actually pooling the dvs together, pairs by pairs
  for (auto &g : dvs_inx) {
    pooled_dvs.push_back(pool(experiment, g));

    // updating the id; otherwise, it's Lua would have problem with filtering
    pooled_dvs.back().id_ = ng++;
  }

  return pooled_dvs;
}

///
/// Pools a pair of dependent variables from different conditions
///
/// @param      experiment  The experiment
/// @param      dvs_inx          The indices of pairs of dependent variables
///
/// @return     Returns the pooled dependent variables across different
///             conditions
///
DependentVariable GroupPooling::pool(Experiment *experiment,
                                     std::vector<int> &dvs_inx) {

  DependentVariable grouped_dv;

  // pooling related dvs together across conditions
  for (auto &g : dvs_inx) {
    grouped_dv.addNewMeasurements(experiment->dvs_[g].measurements());
  }

  return grouped_dv;
}
