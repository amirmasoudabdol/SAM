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

  arma::mat sample(experiment->setup.ng(), experiment->setup.nobs().max());

  // Refactor Me!
  if (params.m_meas_dist) { // Multivariate Distributions
    sample.each_col([this](arma::vec &v) {
      v = Random::get(this->params.m_meas_dist.value());
    });
  } else {
    if (params.meas_dists) {

      sample.each_row([this, i = 0](arma::rowvec &v) mutable {
        v.imbue(
            [&]() { return Random::get(this->params.meas_dists.value()[i]); });
        i++;
      });
    }
  }

  // Adding some noise if specified
  if (params.erro_dist) { // Multivariate Distributions
    arma::mat errors(experiment->setup.ng(), experiment->setup.nobs().max());
    errors.each_col(
        [this](arma::vec &v) { v = Random::get(params.erro_dist.value()); });
    sample += errors;
  } else {
    if (params.erro_dists) {
      arma::mat errors(experiment->setup.ng(), experiment->setup.nobs().max());

      errors.each_row([this, i = 0](arma::rowvec &v) mutable {
        v.imbue(
            [&]() { return Random::get(this->params.erro_dists.value()[i]); });
        i++;
      });
      sample += errors;
    }
  }

  // This is ugly but it should work
  for (int g{0}; g < experiment->setup.ng(); ++g) {
    (*experiment)[g].set_measurements(
        sample.row(g).head(experiment->setup.nobs()[g]));
  }
}

std::vector<arma::Row<double>>
LinearModelStrategy::genNewObservationsForAllGroups(Experiment *experiment,
                                                    int n_new_obs) {

  arma::mat sample(experiment->setup.ng(), n_new_obs);

  if (params.m_meas_dist) { // Multivariate Distributions
    sample.each_col([this](arma::vec &v) {
      v = Random::get(this->params.m_meas_dist.value());
    });
  } else {
    if (params.meas_dists) {

      sample.each_row([this, i = 0](arma::rowvec &v) mutable {
        v.imbue(
            [&]() { return Random::get(this->params.meas_dists.value()[i]); });
        i++;
      });
    }
  }

  // Adding some noise if specified
  if (params.erro_dist) { // Multivariate Distributions
    arma::mat errors(experiment->setup.ng(), experiment->setup.nobs().max());
    errors.each_col(
        [this](arma::vec &v) { v = Random::get(params.erro_dist.value()); });
    sample += errors;
  } else {
    if (params.erro_dists) {
      arma::mat errors(experiment->setup.ng(), experiment->setup.nobs().max());

      errors.each_row([this, i = 0](arma::rowvec &v) mutable {
        v.imbue(
            [&]() { return Random::get(this->params.erro_dists.value()[i]); });
        i++;
      });
      sample += errors;
    }
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
