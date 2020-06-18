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

  /// result flag
  int res = 0;

  for (const auto d : params.multipliers) {

    for (int t = 0;
         t < params.n_attempts && t < params.max_attempts && res != 1; ++t) {

      res = this->removeOutliers(experiment, params.num, d);

      experiment->calculateStatistics();
      experiment->calculateEffects();
      experiment->runTest();

//      if (!decisionStrategy->verdict(*experiment, DecisionStage::WhileHacking)
//               .isStillHacking())
//        return;
    }
  }
  
  for (int g{0}; g < experiment->setup.ng(); ++g) {
    spdlog::debug("\t {}", (*experiment)[g]);
  }
}

int OutliersRemoval::removeOutliers(Experiment *experiment, const int n,
                                    const double d) {

  int res = 0;

  arma::rowvec standaraized;

  /// Removing outliers only from the original groups, see #104
  /// This is addressing the GroupPooling, and it doesn't have to do anything
  /// with outlier removal outlier removal is still being done in all groups
  for (int i = 0; i < experiment->setup.ng(); ++i) {

    auto &row = (*experiment)[i].measurements();

    // At least one row has less than `min_observations`
    if (row.size() <= params.min_observations)
      return 1; // Unsuccessful retrun, nothing has removed.
                // TODO: I can improve this with `std::optional`

    // This trick makes finding the largest outlier easier. I'll see if I can
    // find a better way
    if (params.order == "max first")
      row = sort(row);

    standaraized =
        arma::abs(row - (*experiment)[i].mean_) / (*experiment)[i].stddev_;

    // Finding the outliers, returning only `n` of them
    arma::uvec inx = arma::find(standaraized > d, n, "first");

    if ((row.n_elem - inx.n_elem) <= params.min_observations)
      inx = inx.head(row.n_elem - params.min_observations);

    (*experiment)[i].del_measurements(inx);
  }

  // Success Code
  return res;
}
