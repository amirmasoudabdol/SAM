//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <iostream>
#include <string>

#include <spdlog/spdlog.h>

#include "DataStrategy.h"
#include "Experiment.h"

using namespace sam;

void LinearModelStrategy::genData(Experiment *experiment) {
  
  arma::mat sample = fillMatrix(params.meas_dists,
                                params.m_meas_dist,
                                experiment->setup.ng(),
                                experiment->setup.nobs().max());

  if (params.m_erro_dist or params.erro_dists) {
    arma::mat errors = fillMatrix(params.erro_dists,
                                  params.m_erro_dist,
                                  experiment->setup.ng(),
                                  experiment->setup.nobs().max());
    sample += errors;
  }

  /// This is ugly but it should work
  for (int g{0}; g < experiment->setup.ng(); ++g) {
    (*experiment)[g].set_measurements(
        sample.row(g).head(experiment->setup.nobs()[g]));
  }
}

std::vector<arma::Row<double>>
LinearModelStrategy::genNewObservationsForAllGroups(Experiment *experiment,
                                                    int n_new_obs) {

  arma::mat sample = fillMatrix(params.meas_dists,
                                params.m_meas_dist,
                                experiment->setup.ng(),
                                n_new_obs);

  if (params.m_erro_dist or params.erro_dists) {
    arma::mat errors = fillMatrix(params.erro_dists,
                                  params.m_erro_dist,
                                  experiment->setup.ng(),
                                  n_new_obs);
    sample += errors;
  }

  std::vector<arma::Row<double>> new_values(experiment->setup.ng());

  std::generate(new_values.begin(), new_values.end(),
                [&, i = 0]() mutable { return sample.row(i++); });

  return new_values;
}

arma::Row<double>
LinearModelStrategy::genNewObservationsFor(Experiment *experiment, int g,
                                           int n_new_obs) {

  // To be implemented still...
  return arma::Row<double>();
}
