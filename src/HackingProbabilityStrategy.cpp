//
// Created by Amir Masoud Abdol on 2020-07-28
//

#include "HackingProbabilityStrategy.h"

#include <boost/math/special_functions/relative_difference.hpp>

using namespace sam;
using boost::math::relative_difference;

HackingProbabilityStrategy::~HackingProbabilityStrategy(){
  // Pure deconstructor
};


std::unique_ptr<HackingProbabilityStrategy>
HackingProbabilityStrategy::build(json &config) {
  
  auto params = config.get<FrankenbachStrategy::Parameters>();
  return std::make_unique<FrankenbachStrategy>(params);
}


double
FrankenbachStrategy::estimate(Experiment *experiment) {
  
  if (relative_difference(params.base_hp, 0) < 0.00001) {
    return 0.;
  }else if (relative_difference(params.base_hp, 1) < 0.00001) {
    return 1.;
  }else {
    /// We have something in the middle now, so, we are calculating based on the p-value
    /// we check for significance, if sig, then we return 0.
    /// else, then we assign a value
    
    bool is_any_non_sig {false};
    for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
         ++i, ++d %= experiment->setup.nd()) {
      is_any_non_sig |= !experiment->groups_[i].sig_;
    }
    
    if (!is_any_non_sig) {
      return false;
    }else {
      /// I have a feeling this is a very inefficneit implementation
      int d = experiment->setup.nd();
      int g = experiment->setup.ng();
      
      arma::rowvec dangers(g - d);
      
      dangers.imbue([&, i = d]() mutable {
        return this->border(experiment->groups_[i++].se_effect_);
      });
      
      probabilities.resize(g - d);
      probabilities.imbue([&, i = d]() mutable {
        if (experiment->groups_[i].effect_ > dangers[i]) {
          
          double d_sig = experiment->groups_[i].se_effect_ * 1.959964;
          arma::rowvec danger_breaks = arma::linspace<arma::rowvec>(dangers[i], d_sig, 11);
          std::cout << danger_breaks;
            
          return arma::as_scalar(hp_range.at(arma::max(find(danger_breaks < experiment->groups_[i++].se_effect_))));
          
        }else{
          return 0.;
        }
      });
      
    }
    
    return arma::max(probabilities);
    
  }
  
}
