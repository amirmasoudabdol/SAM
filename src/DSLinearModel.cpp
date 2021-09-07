//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <string>

#include <spdlog/spdlog.h>

#include "DataStrategy.h"
#include "Experiment.h"

using namespace sam;

void LinearModelStrategy::genData(Experiment *experiment) {
  
  /// Generates the samples
  arma::Mat<float> sample = fillMatrix(params.meas_dists,
                                params.m_meas_dist,
                                experiment->setup.ng(),
                                experiment->setup.nobs().max());
  
  

  /// Generate the error terms if specified
//  if (params.m_erro_dist or params.erro_dists) {
  std::normal_distribution<> norm(0, sqrt(params.tau2));
  float t2 = Random::get(norm);
    params.m_erro_dist = makeMultivariateDistribution({
      {"dist", "mvnorm_distribution"},
      {"means", {0, t2}},
      {"covs", 0},
      {"stddevs", 1}
    });
    
    arma::Mat<float> errors = fillMatrix(params.erro_dists,
                                  params.m_erro_dist,
                                  experiment->setup.ng(),
                                  experiment->setup.nobs().max());
  
    sample += errors;
//  }

  /// This is ugly but it should work
  for (int g{0}; g < experiment->setup.ng(); ++g) {
    (*experiment)[g].setMeasurements(
        sample.row(g).head(experiment->setup.nobs()[g]));
  }
}

std::vector<arma::Row<float>>
LinearModelStrategy::genNewObservationsForAllGroups(Experiment *experiment,
                                                    int n_new_obs) {

  arma::Mat<float> sample = fillMatrix(params.meas_dists,
                                params.m_meas_dist,
                                experiment->setup.ng(),
                                n_new_obs);

  if (params.m_erro_dist or params.erro_dists) {
    arma::Mat<float> errors = fillMatrix(params.erro_dists,
                                  params.m_erro_dist,
                                  experiment->setup.ng(),
                                  n_new_obs);
    sample += errors;
  }

  std::vector<arma::Row<float>> new_values(experiment->setup.ng());

  std::generate(new_values.begin(), new_values.end(),
                [&, i = 0]() mutable { return sample.row(i++); });

  return new_values;
}

