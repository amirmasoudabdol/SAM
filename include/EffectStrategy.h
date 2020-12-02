//
// Created by Amir Masoud Abdol on 2019-01-24.
//

///
/// \defgroup   EffectStrategies Effect Strategies
/// \brief      List of available effect strategies
///

#ifndef SAMPP_EFFECTSTRATEGY_H
#define SAMPP_EFFECTSTRATEGY_H

#include "sam.h"

namespace sam {

class Experiment;
class ExperimentSetup;

///
/// \brief      Abstract class for Effect Size Strategy
///
class EffectStrategy {

public:
  enum class EffectEstimator { CohensD, HedgesG, OddRatio, StandardizedMeanDifference };

  struct EffectStrategyParameters {
    EffectEstimator name;
  };

  EffectStrategyParameters params;

  static std::unique_ptr<EffectStrategy> build(json &effect_strategy_config);

  virtual ~EffectStrategy() = 0;

  virtual void computeEffects(Experiment *experiment) = 0;

  // std::string name = "";
};

///
/// @ingroup  EffectStrategies
///
class MeanDifference final : public EffectStrategy {
  
public:
  //        std::string name = "MeanDifference";
  
  explicit MeanDifference(){
    // name = "MeanDifference";
  };
  
  explicit MeanDifference(EffectStrategyParameters esp) { params = esp; };
  
  void computeEffects(Experiment *experiment);
};

///
/// @ingroup  EffectStrategies
///
class StandardizedMeanDifference final : public EffectStrategy {

public:
  //        std::string name = "StandardizedMeanDifference";

  explicit StandardizedMeanDifference(){
      // name = "StandardizedMeanDifference";
  };

  explicit StandardizedMeanDifference(EffectStrategyParameters esp) { params = esp; };

  void computeEffects(Experiment *experiment);
};

///
/// @ingroup  EffectStrategies
///
class CohensD final : public EffectStrategy {

public:
  //        std::string name = "CohensD";

  explicit CohensD(){
      // name = "CohensD";
  };

  explicit CohensD(EffectStrategyParameters esp) { params = esp; };

  void computeEffects(Experiment *experiment);
};

///
/// @ingroup  EffectStrategies
///
class HedgesG final : public EffectStrategy {

public:
  //        std::string name = "HedgesG";

  explicit HedgesG() {
    // name = "HedgesG";
  }

  explicit HedgesG(EffectStrategyParameters esp) { params = esp; };

  void computeEffects(Experiment *experiment);
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
