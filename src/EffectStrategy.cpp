//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <iostream>

#include "EffectStrategy.h"
#include "Experiment.h"

using namespace sam;

EffectStrategy::~EffectStrategy(){
    // Pure deconstructor
};

std::unique_ptr<EffectStrategy>
EffectStrategy::build(json &effect_strategy_config) {
  if (effect_strategy_config["_name"] == "CohensD") {
    return std::make_unique<CohensD>();
  } else if (effect_strategy_config["_name"] == "HedgesG") {
    return std::make_unique<HedgesG>();
  } else if (effect_strategy_config["_name"] == "MeanDifference") {
    return std::make_unique<MeanDifference>();
  } else {
    throw std::invalid_argument("Uknown effect size strategy.\n");
  }
}

void CohensD::computeEffects(Experiment *experiment) {

  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       i++, d %= experiment->setup.nd()) {
    (*experiment)[i].effect_ =
        cohens_d((*experiment)[d].mean_, (*experiment)[d].stddev_,
                 (*experiment)[d].nobs_, (*experiment)[i].mean_,
                 (*experiment)[i].stddev_, (*experiment)[i].nobs_);
  }
}

void HedgesG::computeEffects(Experiment *experiment) {

  /// Skipping Treatment groups
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       i++, d %= experiment->setup.nd()) {
    (*experiment)[i].effect_ =
        hedges_g((*experiment)[d].mean_, (*experiment)[d].stddev_,
                 (*experiment)[d].nobs_, (*experiment)[i].mean_,
                 (*experiment)[i].stddev_, (*experiment)[i].nobs_);
  }
}

void MeanDifference::computeEffects(Experiment *experiment) {
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       i++, d %= experiment->setup.nd()) {
    (*experiment)[i].effect_ =
        mean_difference((*experiment)[i].mean_, (*experiment)[i].stddev_,
                        (*experiment)[d].mean_, (*experiment)[d].stddev_);
  }
}

namespace sam {

double mean_difference(double Sm1, double Sd1, double Sm2, double Sd2) {
  return (Sm1 - Sm2) / sqrt(0.5 * (pow(Sd1, 2) + pow(Sd2, 2)));
}

double cohens_d(double Sm1, double Sd1, double Sn1, double Sm2, double Sd2,
                double Sn2) {

  // Degrees of freedom:
  double df = Sn1 + Sn2 - 2;

  // Pooled variance and hence standard deviation:
  double sp = sqrt(((Sn1 - 1) * Sd1 * Sd1 + (Sn2 - 1) * Sd2 * Sd2) / df);

  // Cohen's D:
  return std::abs(Sm1 - Sm2) / sp;
}

double hedges_g(double Sm1, double Sd1, double Sn1, double Sm2, double Sd2,
                double Sn2) {

  // Degrees of freedom:
  double df = Sn1 + Sn2 - 2;

  // Pooled variance and hence standard deviation:
  double sp = sqrt(((Sn1 - 1) * Sd1 * Sd1 + (Sn2 - 1) * Sd2 * Sd2) / df);

  // Cohen's D:
  double Cd = std::abs(Sm1 - Sm2) / sp;

  // Sum of sample sizes
  double n = Sn1 + Sn2;

  // Adding the bais correction factor for n < 50
  if (n < 50) {
    return Cd * (n - 3) / (n - 2.25) * sqrt((n - 2) / n);
  }

  return Cd;
  //    return Cd / sqrt(Sn / df);
}

double pearsons_r(double Cd) { return Cd / sqrt(Cd * Cd + 4); }

double glass_delta(double Sm1, double Sd1, double Sn1, double Sm2, double Sd2,
                   double Sn2) {
  return (Sm1 - Sm2) / Sd2;
}

} // namespace sam
