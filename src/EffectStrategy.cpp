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
    
    (*experiment)[i].effect_ = res.est;
    (*experiment)[i].effect_var = res.var;
  }
}

void HedgesG::computeEffects(Experiment *experiment) {

  /// Skipping Treatment groups
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {
    auto res = hedges_g((*experiment)[i].mean_, (*experiment)[i].stddev_,
                        (*experiment)[i].nobs_, (*experiment)[d].mean_,
                        (*experiment)[d].stddev_, (*experiment)[d].nobs_);
    
    (*experiment)[i].effect_ = res.est;
    (*experiment)[i].effect_var = res.var;
        
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
    auto res =
        standardized_mean_difference((*experiment)[i].mean_, (*experiment)[i].stddev_,
                        (*experiment)[d].mean_, (*experiment)[d].stddev_);
    
    (*experiment)[i].effect_ = res.est;
    (*experiment)[i].effect_var = res.var;
  }
}

namespace sam {

float mean_difference(float Sm1, float Sm2) {
  return (Sm1 - Sm2);
}

StandardizedMeanDifference::ResultType
StandardizedMeanDifference::standardized_mean_difference(float Sm1, float Sd1, float Sm2, float Sd2) {
  float var = 0.5 * (pow(Sd1, 2) + pow(Sd2, 2));
  return {(Sm1 - Sm2) / sqrt(var), var};
}

CohensD::ResultType
CohensD::cohens_d(float Sm1, float Sd1, float Sn1, float Sm2, float Sd2,
                float Sn2) {

  // Degrees of freedom:
  float df = Sn1 + Sn2 - 2;

  // Pooled variance and hence standard deviation:
  float var_d = ((Sn1 - 1) * Sd1 * Sd1 + (Sn2 - 1) * Sd2 * Sd2) / df;
  
  // Cohen's D
  float d = (Sm1 - Sm2) / sqrt(var_d);

  return {d, var_d};
}

HedgesG::ResultType
HedgesG::hedges_g(float Sm1, float Sd1, float Sn1, float Sm2, float Sd2,
                float Sn2) {
  
  // Cohen's D
  auto Cd = CohensD::cohens_d(Sm1, Sd1, Sn1, Sm2, Sd2, Sn2);
  
  // Hedges' adjustment
  float J = 1 - (3 / (4 * (Sn1 + Sn2) - 9));

  return {J * Cd.est, Cd.var};
}

float pearsons_r(float Cd) { return Cd / sqrt(Cd * Cd + 4); }

float glass_delta(float Sm1, float Sd1, float Sn1, float Sm2, float Sd2,
                   float Sn2) {
  return (Sm1 - Sm2) / Sd2;
}

} // namespace sam
