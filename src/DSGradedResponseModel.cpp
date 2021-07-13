//
// Created by Amir Masoud Abdol on 2019-10-29
//

#include "DataStrategy.h"
#include "Experiment.h"
#include "ExperimentSetup.h"

using namespace sam;

GRMDataStrategy::GRMDataStrategy(const Parameters &p) : params(p) {
  /// Some initialization
  betas.resize(params.n_items, params.n_categories - 1);
  poa.resize(params.n_items, params.n_categories - 1);
  responses.resize(params.n_items, params.n_categories - 1);
  urand.resize(params.n_items, params.n_categories - 1);
  scores.resize(params.n_items);
}

void GRMDataStrategy::genData(Experiment *experiment) {
  
  betas = fillMatrix(params.diff_dists, params.m_diff_dist,
                     params.n_categories - 1,
                     params.n_items);
  
  // Sorting β's
  betas = arma::sort(betas, "ascend", 0);
  
  // This is a strange hack, I should redo the GRM totally
  arma::inplace_trans(betas);
  
  auto thetas = fillMatrix(params.abil_dists, params.m_abil_dist,
                           experiment->setup.ng(),
                           experiment->setup.nobs().max());
  
  for (int g{0}; g < experiment->setup.ng(); ++g) {

    arma::Row<float> data(experiment->setup.nobs()[g]);

    data.imbue([&, i = 0]() mutable {
      return rasch_score(thetas.at(i++, g));
    });

    (*experiment)[g].setMeasurements(data);
  }
}

// Generate persons j answer to all items
float GRMDataStrategy::rasch_score(const float theta) {

  poa = arma::exp(theta - betas);
  poa = poa / (1 + poa);

  // This is a super slow process, and it's been replaced by `arma::randu()`.
  // urand.imbue([&]() { return Random::get<float>(0., 1.); });
  
  // this is much better but then I'm out of my RNG chain, which should kind of be fine,
  // because they are just some random values for evaluation,
  urand.randu();    /// @note this requires arma::arma_rng::set_seed(); to work

  responses = urand < poa;
  responses.insert_cols(params.n_categories - 1, arma::uvec(params.n_items, arma::fill::zeros));
  
  scores.imbue([&, i = 0]() mutable {
    arma::uvec zeros = arma::find((responses.row(i++)) == 0);
    return zeros.empty() ? params.n_categories : zeros.at(0);
  });
  scores += 1;
  
  return arma::accu(scores);
}

std::vector<arma::Row<float>>
GRMDataStrategy::genNewObservationsForAllGroups(Experiment *experiment,
                                                int n_new_obs) {

  std::vector<arma::Row<float>> new_values(experiment->setup.ng());

  for (int g{0}; g < experiment->setup.ng(); ++g) {
    new_values[g].resize(n_new_obs);

    auto thetas = fillMatrix(params.abil_dists, params.m_abil_dist,
                       experiment->setup.ng(),
                       n_new_obs);
    
    new_values[g].imbue([&, i = 0]() mutable {
      return rasch_score(thetas.at(i++, g));
    });
  }

  return new_values;
}

