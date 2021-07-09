//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <iostream>

#include "EffectStrategy.h"
#include "Experiment.h"

using namespace sam;

EffectStrategy::~EffectStrategy(){
    // Pure destructors
}

std::unique_ptr<EffectStrategy>
EffectStrategy::build(json &effect_strategy_config) {
  if (effect_strategy_config["name"] == "CohensD") {
    return std::make_unique<CohensD>();
  } else if (effect_strategy_config["name"] == "HedgesG") {
    return std::make_unique<HedgesG>();
  } else if (effect_strategy_config["name"] == "MeanDifference") {
    return std::make_unique<MeanDifference>();
  } else if (effect_strategy_config["name"] == "StandardizedMeanDifference") {
    return std::make_unique<StandardizedMeanDifference>();
  } else {
    spdlog::critical("Unknown effect size strategy.");
    exit(1);
  }
}

void CohensD::computeEffects(Experiment *experiment) {

  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {
    auto res =
        cohens_d((*experiment)[i].mean_, (*experiment)[i].stddev_,
                 (*experiment)[i].nobs_, (*experiment)[d].mean_,
                 (*experiment)[d].stddev_, (*experiment)[d].nobs_);
    (*experiment)[i].effect_ = std::get<0>(res);
    (*experiment)[i].var_effect_ = std::get<1>(res);
    (*experiment)[i].se_effect_ = std::get<2>(res);
  }
}

void HedgesG::computeEffects(Experiment *experiment) {

  /// Skipping Treatment groups
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {
    (*experiment)[i].effect_ =
        hedges_g((*experiment)[i].mean_, (*experiment)[i].stddev_,
                 (*experiment)[i].nobs_, (*experiment)[d].mean_,
                 (*experiment)[d].stddev_, (*experiment)[d].nobs_);
  }
}

void MeanDifference::computeEffects(Experiment *experiment) {
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {
    (*experiment)[i].effect_ =
    mean_difference((*experiment)[i].mean_, (*experiment)[d].mean_);
  }
}

void StandardizedMeanDifference::computeEffects(Experiment *experiment) {
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {
    (*experiment)[i].effect_ =
        standardized_mean_difference((*experiment)[i].mean_, (*experiment)[i].stddev_,
                        (*experiment)[d].mean_, (*experiment)[d].stddev_);
  }
}

namespace sam {

float mean_difference(float Sm1, float Sm2) {
  return (Sm1 - Sm2);
}

float standardized_mean_difference(float Sm1, float Sd1, float Sm2, float Sd2) {
  return (Sm1 - Sm2) / sqrt(0.5 * (pow(Sd1, 2) + pow(Sd2, 2)));
}

std::tuple<float, float, float> cohens_d(float Sm1, float Sd1, float Sn1, float Sm2, float Sd2,
                float Sn2) {

  // Degrees of freedom:
  float df = Sn1 + Sn2 - 2;

  // Pooled variance and hence standard deviation:
  float sp = sqrt(((Sn1 - 1) * Sd1 * Sd1 + (Sn2 - 1) * Sd2 * Sd2) / df);
  
  float d = (Sm1 - Sm2) / sp;
  float var_d = ((Sn1 + Sn2) / (Sn1 * Sn2) + pow(d, 2) / (2 * df)) * ((Sn1 + Sn2) / (df));
  float se_d = sqrt(var_d);

  // Cohen's D:
  return std::make_tuple(d, var_d, se_d);
}

float hedges_g(float Sm1, float Sd1, float Sn1, float Sm2, float Sd2,
                float Sn2) {
  
  std::cout << "Test me first...\n";

  // Degrees of freedom:
  float df = Sn1 + Sn2 - 2;

  // Pooled variance and hence standard deviation:
  float sp = sqrt(((Sn1 - 1) * Sd1 * Sd1 + (Sn2 - 1) * Sd2 * Sd2) / df);

  // Cohen's D:
  float Cd = std::abs(Sm1 - Sm2) / sp;

  // Sum of sample sizes
  float n = Sn1 + Sn2;

  // Adding the bias correction factor for n < 50
  if (n < 50) {
    return Cd * (n - 3) / (n - 2.25) * sqrt((n - 2) / n);
  }

  return Cd;
  //    return Cd / sqrt(Sn / df);
}

float pearsons_r(float Cd) { return Cd / sqrt(Cd * Cd + 4); }

float glass_delta(float Sm1, float Sd1, float Sn1, float Sm2, float Sd2,
                   float Sn2) {
  return (Sm1 - Sm2) / Sd2;
}

} // namespace sam
