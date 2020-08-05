//
// Created by Amir Masoud Abdol on 2020-07-28
//

#include "sam.h"
#include "Experiment.h"
#include <boost/math/distributions/normal.hpp>

namespace sam {

class HackingProbabilityStrategy {
  
public:
  
  double prob;
  arma::rowvec probabilities;
  
  virtual ~HackingProbabilityStrategy() = 0;
  
  static std::unique_ptr<HackingProbabilityStrategy> build(json &config);
  
  virtual double estimate(Experiment *experiment) = 0;
  
  operator double() {
    return prob;
  }
  
  operator arma::rowvec() {
    return probabilities;
  }
};


class FrankenbachStrategy final : public HackingProbabilityStrategy {
  
  arma::rowvec se_seq;
  arma::rowvec p_seq;
  arma::rowvec se_seq_p;
  arma::rowvec hackborder;
  arma::rowvec hp_range;
  
  /// Left border of the danger zone
  double border(double se_d) {
    return arma::as_scalar(hackborder.at(arma::max(find(se_seq < se_d))));
  }

public:
  struct Parameters {
    std::string method {"FrankenbachStrategy"};
    double base_hp {0};
    double lo_se {0.4};
    double hi_se {0.6};
    double min_se {0.1};
    double max_se {0.6};
  };
  
  Parameters params;
  
  FrankenbachStrategy() = default;
  
  FrankenbachStrategy(Parameters &p) : params{p} {
    
    using boost::math::normal; normal norm;
    
    se_seq = arma::regspace<arma::rowvec>(params.min_se, 0.0001, params.max_se);
    p_seq = arma::linspace<arma::rowvec>(params.lo_se + 0.0001, params.hi_se - 0.0001, se_seq.n_elem);
    
    se_seq_p.resize(p_seq.n_elem);
    se_seq_p.imbue([&, i = 0]() mutable {
      return quantile(norm, 1. - p_seq[i++]/ 2.);
    });
    
    hackborder = se_seq_p % se_seq;
    hackborder = arma::sort(hackborder);
    
    hp_range = arma::linspace<arma::rowvec>(params.base_hp - 0.2, params.base_hp + 0.2 - 0.0001, 11);
  };
  
  virtual double estimate(Experiment *experiment) override;
  
};


inline void to_json(json &j, const FrankenbachStrategy::Parameters &p) {
  j = json{{"method", p.method},
    {"base_hp", p.base_hp},
    {"lo_se", p.lo_se},
    {"hi_se", p.hi_se},
    {"min_se", p.min_se},
    {"max_se", p.max_se}
  };
}

inline void from_json(const json &j, FrankenbachStrategy::Parameters &p) {
  
  j.at("method").get_to(p.method);
  j.at("base_hp").get_to(p.base_hp);
  j.at("lo_se").get_to(p.lo_se);
  j.at("hi_se").get_to(p.hi_se);
  j.at("min_se").get_to(p.min_se);
  j.at("max_se").get_to(p.max_se);
}


};
