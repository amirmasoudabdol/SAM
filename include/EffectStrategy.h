//
// Created by Amir Masoud Abdol on 2019-01-24.
//

///
/// @defgroup   EffectStrategies Effect Strategies
/// @brief      List of available effect strategies
///

#ifndef SAMPP_EFFECTSTRATEGY_H
#define SAMPP_EFFECTSTRATEGY_H

#include "sam.h"

namespace sam {

class Experiment;
class ExperimentSetup;

///
/// @brief      Abstract class for Effect Size Strategy
///
class EffectStrategy {

public:
  enum class EffectEstimator { CohensD, HedgesG, OddRatio, StandardizedMeanDifference };

  static std::unique_ptr<EffectStrategy> build(json &effect_strategy_config);

  virtual ~EffectStrategy() = 0;

  virtual void computeEffects(Experiment *experiment) = 0;

};

///
/// @ingroup  EffectStrategies
///
class MeanDifference final : public EffectStrategy {
  
public:
  
  explicit MeanDifference() = default;
  
  void computeEffects(Experiment *experiment) override;
};

///
/// @ingroup  EffectStrategies
///
class StandardizedMeanDifference final : public EffectStrategy {

public:

  explicit StandardizedMeanDifference() = default;

  void computeEffects(Experiment *experiment) override;
};

///
/// @ingroup  EffectStrategies
///
class CohensD final : public EffectStrategy {

public:

  explicit CohensD() = default;

  void computeEffects(Experiment *experiment) override;
};

///
/// @ingroup  EffectStrategies
///
class HedgesG final : public EffectStrategy {

public:

  explicit HedgesG() = default;

  void computeEffects(Experiment *experiment) override;
};

//
// class OddRatio final : public EffectStrategy {
//
// public:
//    void computeEffects(Experiment *experiment);
//
// private:
//    const std::string name = "OddRatio";
//};
//
//
// class PearsonR final : public EffectStrategy {
//
// public:
//    void computeEffects(Experiment *experiment);
//
// private:
//    const std::string name = "PearsonR";
//};
//
// class GlassDelta final : public EffectStrategy {
//
// public:
//    void computeEffects(Experiment *experiment);
//
// private:
//    const std::string name = "GlassDelta";
//};
//
// class EtaSquared final : public EffectStrategy {
//
// public:
//    void computeEffects(Experiment *experiment);
//
// private:
//    const std::string name = "EtaSquared";
//};

double mean_difference(double Sm1, double Sm2);

double standardized_mean_difference(double Sm1, double Sd1, double Sm2, double Sd2);

void cohens_d(Experiment *expr);

std::tuple<double, double, double>
cohens_d(double Sm1, double Sd1, double Sn1, double Sm2, double Sd2,
                double Sn2);
double hedges_g(double Sm1, double Sd1, double Sn1, double Sm2, double Sd2,
                double Sn2);
double pearsons_r(double Cd);
double glass_delta(double Sm1, double Sd1, double Sn1, double Sm2, double Sd2,
                   double Sn2);

} // namespace sam

#endif // SAMPP_EFFECTSTRATEGY_H
