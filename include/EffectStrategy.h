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

float mean_difference(float Sm1, float Sm2);

float standardized_mean_difference(float Sm1, float Sd1, float Sm2, float Sd2);

void cohens_d(Experiment *expr);

std::tuple<float, float, float>
cohens_d(float Sm1, float Sd1, float Sn1, float Sm2, float Sd2,
                float Sn2);
float hedges_g(float Sm1, float Sd1, float Sn1, float Sm2, float Sd2,
                float Sn2);
float pearsons_r(float Cd);
float glass_delta(float Sm1, float Sd1, float Sn1, float Sm2, float Sd2,
                   float Sn2);

} // namespace sam

#endif // SAMPP_EFFECTSTRATEGY_H
