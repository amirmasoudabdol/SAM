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
  
  
  if (params.tau2 != 0.) {

    MultivariateDistribution mu_noiser = makeMultivariateDistribution({
      {"dist", "mvnorm_distribution"},
      {"means", arma::conv_to<std::vector<float>>::from(params.means)},
      {"covs", 0},
      {"stddevs", sqrt(params.tau2)}
    });
    
    arma::Col<float> noisy_means = Random::get(mu_noiser);

    
    MultivariateDistribution tau2_bar = makeMultivariateDistribution({
      {"dist", "mvnorm_distribution"},
      {"means", arma::conv_to<std::vector<float>>::from(noisy_means)},
      {"covs", 0},
      {"stddevs", 1}
    });
    
    arma::Mat<float> random_effect_error(experiment->setup.ng(),
                                         experiment->setup.nobs().max());
    random_effect_error.each_col([&](arma::Col<float> &v) {
      v = Random::get(tau2_bar);
    });
    
    auto t_inxs = experiment->setup.ng() - experiment->setup.nd();

    sample.tail_rows(t_inxs) += random_effect_error.tail_rows(t_inxs);
  }

  /// Generate the error terms if specified
  if (params.m_erro_dist or params.erro_dists) {
    arma::Mat<float> errors = fillMatrix(params.erro_dists,
                                  params.m_erro_dist,
                                  experiment->setup.ng(),
                                  experiment->setup.nobs().max());
    sample += errors;
  }

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

