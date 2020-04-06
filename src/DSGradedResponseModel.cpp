//
// Created by Amir Masoud Abdol on 2019-10-29
//

#include <iostream>
#include <string>

#include "DataStrategy.h"
#include "Experiment.h"
#include "ExperimentSetup.h"

using namespace sam;

GRMDataStrategy::GRMDataStrategy(const Parameters &p) : params(p) {
  /// Some initialization
  betas.resize(params.n_items, params.n_categories);
  poa.resize(params.n_items, params.n_categories);
  responses.resize(params.n_items, params.n_categories);
  urand.resize(params.n_items, params.n_categories);
}

void GRMDataStrategy::genData(Experiment *experiment) {
  
  betas = fillMatrix(params.diff_dists, params.m_diff_dist,
                     params.n_items,
                     params.n_categories);

  for (int g{0}; g < experiment->setup.ng(); ++g) {

    arma::Row<double> data(experiment->setup.nobs()[g]);

    static double theta{0};
    do {
      
      /// This is somewhat better, but it's still not perfect
      /// TODO: I should find a way to move this out of the for-loop
      auto thetas = fillMatrix(params.abil_dists, params.m_abil_dist,
                               experiment->setup.ng(),
                               experiment->setup.nobs().max());

      data.imbue([&, i = 0]() mutable {
        return generate_sum_of_scores(thetas.at(i++, g));
      });

      // This makes sure that I don't have a totally unanswered test
    } while (!arma::any(data));

    (*experiment)[g].set_measurements(data);
  }
}

// Generate persons j answer to all items
double GRMDataStrategy::generate_sum_of_scores(const double theta) {

  poa = (arma::exp(theta - betas)) / (1 + arma::exp(theta - betas));

  urand.imbue([&]() { return Random::get(uniform_dist); });

  responses = poa > urand;

  return arma::accu(responses);
}

std::vector<arma::Row<double>>
GRMDataStrategy::genNewObservationsForAllGroups(Experiment *experiment,
                                                int n_new_obs) {

  std::vector<arma::Row<double>> new_values(experiment->setup.ng());

  for (int g{0}; g < experiment->setup.ng(); ++g) {
    new_values[g].resize(n_new_obs);

    auto thetas = fillMatrix(params.abil_dists, params.m_abil_dist,
                       experiment->setup.ng(),
                       n_new_obs);
    
    new_values[g].imbue([&, i = 0]() mutable {
      return generate_sum_of_scores(thetas.at(i++, g));
    });
  }

  return new_values;
}

arma::Row<double> GRMDataStrategy::genNewObservationsFor(Experiment *experiment,
                                                         int g, int n_new_obs) {
  return arma::Row<double>();
}
