//
// Created by Amir Masoud Abdol on 18/03/2020.
//

#include "HackingStrategy.h"

using namespace sam;

/**
 Perform subjective outliers removal on an Experiment. There is not much
 difference between this routine and the OutliersRemoval, mainly, I generate the
 Ks beforehand, and the entire procedure is technically one hacking step.

 */
void SubjectiveOutlierRemoval::perform(Experiment *experiment,
                                       DecisionStrategy *decision_strategy) {

  spdlog::debug("Subjective Outliers Removal");

  /// Creating a regularly spaced vector of values with the given start, step,
  /// end.
  static arma::vec Ks = arma::regspace<arma::vec>(
      params.range[1], -1. * params.step_size, params.range[0]);

  /// Going through K's in order and return as soon as the deicsion_strategy is
  /// satisfied.
  for (const auto &k : Ks) {

    /// Removing the outliers from control groups as well.
    for (int i {0}; i < experiment->setup.ng(); ++i) {
      auto &row = (*experiment)[i].measurements();

      arma::rowvec standaraized =
          arma::abs(row - (*experiment)[i].mean_) / (*experiment)[i].stddev_;

      /// Finding the outliers
      arma::uvec inx = arma::find(standaraized > k);

      /// Keeping at least `min_observations`
      if ((row.n_elem - inx.n_elem) <= params.min_observations)
        inx = inx.head(row.n_elem - params.min_observations);

      (*experiment)[i].del_measurements(inx);
    }

    experiment->calculateStatistics();
    experiment->calculateEffects();
    experiment->runTest();

    if (!decision_strategy->verdict(*experiment, DecisionStage::WhileHacking)
             .isStillHacking()) {
      return;
    }
  }
}
