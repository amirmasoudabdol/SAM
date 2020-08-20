//
// Created by Amir Masoud Abdol on 18/03/2020.
//

#include "HackingStrategy.h"

using namespace sam;

///
/// \brief      Implementation of Outliers Removal based on an item's distance
/// from
///             the \mu.
///
void OutliersRemoval::perform(Experiment *experiment) {

  spdlog::debug("Outliers Removal: ");

  for (const auto k : params.multipliers) {
    
    bool res {true};

    /// Removing outliers `n` at a time, for the total of `n_attempts`
    /// It'll stop either when n_attempts are exhausted, or there is no
    /// more observations left to be removed
    for (int t = 0; t < params.n_attempts && res; ++t) {

      res = this->removeOutliers(experiment, params.num, k);
      
      experiment->recalculateEverything();
      
      if(!params.stopping_cond_defs.empty()) {
        if (stopping_condition(experiment)) {
          spdlog::trace("⚠️ Stopping the hacking procedure, stopping condition has been met!");
          return;
        }
      }

    }
  }
  
  for (int g{0}; g < experiment->setup.ng(); ++g) {
    spdlog::debug("\t {}", (*experiment)[g]);
  }
}

bool OutliersRemoval::removeOutliers(Experiment *experiment, const int n, const double k) {

  arma::rowvec standaraized;

  /// Removing outliers only from the original groups, see #104
  /// This is addressing the GroupPooling, and it doesn't have to do anything
  /// with outlier removal outlier removal is still being done in all groups
  for (int i = 0; i < experiment->setup.ng(); ++i) {

    auto &row = (*experiment)[i].measurements();

    // At least one row has less than `min_observations`
    if (row.size() <= params.min_observations)
      return false; // Unsuccessful retrun, nothing has removed.

    // This trick makes finding the largest outlier easier. I'll see if I can
    // find a better way
    if (params.order == "max first")
      row = sort(row);

    standaraized =
        arma::abs(row - (*experiment)[i].mean_) / (*experiment)[i].stddev_;

    // Finding the outliers, returning only `n` of them
    arma::uvec inx = arma::find(standaraized > k, n, "first");

    if ((row.n_elem - inx.n_elem) <= params.min_observations)
      inx = inx.head(row.n_elem - params.min_observations);

    (*experiment)[i].del_measurements(inx);
  }

  // Success Code
  return true;
}
