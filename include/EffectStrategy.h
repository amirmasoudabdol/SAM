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
  
  struct ResultType {
    float est;
    float var;
    
    ResultType() = default;
    
    friend std::ostream &operator<<(std::ostream &os, const ResultType &type) {
      os << "smd" << type.est
         << "smd_var" << type.var;
      return os;
    }
    
    static std::vector<std::string> Columns() {
      return {"smd", "smd_var"};
    }
    
    operator std::vector<std::string>() {
      return {
        std::to_string(est),
        std::to_string(var)
      };
    }
    
    operator std::map<std::string, std::string>() {
      return {
        {"smd", std::to_string(est)},
        {"smd_var", std::to_string(var)}
      };
    }
    
    operator arma::Row<float>() {
      return {
        est,
        var
      };
    }
  };

  explicit StandardizedMeanDifference() = default;

  void computeEffects(Experiment *experiment) override;
  
  static ResultType standardized_mean_difference(float Sm1, float Sd1, float Sm2, float Sd2);
};

///
/// @ingroup  EffectStrategies
///
class CohensD final : public EffectStrategy {

public:
  
  struct ResultType {
    float est;
    float var;
    
    ResultType() = default;
    
    friend std::ostream &operator<<(std::ostream &os, const ResultType &type) {
      os << "cohens_d" << type.est
      << "cohens_d_var" << type.var;
      return os;
    }
    
    static std::vector<std::string> Columns() {
      return {"cohens_d", "cohens_d_var"};
    }
    
    operator std::vector<std::string>() {
      return {
        std::to_string(est),
        std::to_string(var)
      };
    }
    
    operator std::map<std::string, std::string>() {
      return {
        {"cohens_d", std::to_string(est)},
        {"cohens_d_var", std::to_string(var)}
      };
    }
    
    operator arma::Row<float>() {
      return {
        est,
        var
      };
    }
  };

  explicit CohensD() = default;

  void computeEffects(Experiment *experiment) override;
  
  static ResultType cohens_d(float Sm1, float Sd1, float Sn1, float Sm2, float Sd2,
                             float Sn2);
};

///
/// @ingroup  EffectStrategies
///
class HedgesG final : public EffectStrategy {

public:
  
  struct ResultType {
    float est;
    float var;
    
    ResultType() = default;
    
    friend std::ostream &operator<<(std::ostream &os, const ResultType &type) {
      os << "hedges_g" << type.est
      << "hedges_g_var" << type.var;
      return os;
    }
    
    static std::vector<std::string> Columns() {
      return {"hedges_g", "hedges_var"};
    }
    
    operator std::vector<std::string>() {
      return {
        std::to_string(est),
        std::to_string(var)
      };
    }
    
    operator std::map<std::string, std::string>() {
      return {
        {"hedges_g", std::to_string(est)},
        {"hedges_g_var", std::to_string(var)}
      };
    }
    
    operator arma::Row<float>() {
      return {
        est,
        var
      };
    }
  };

  explicit HedgesG() = default;

  void computeEffects(Experiment *experiment) override;
  
  static ResultType hedges_g(float Sm1, float Sd1, float Sn1, float Sm2, float Sd2,
                             float Sn2);
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
