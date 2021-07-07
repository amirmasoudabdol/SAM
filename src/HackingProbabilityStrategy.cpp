//
// Created by Amir Masoud Abdol on 2020-07-28
//

#include "HackingProbabilityStrategy.h"

#include <boost/math/special_functions/relative_difference.hpp>

using namespace sam;
using boost::math::relative_difference;

HackingProbabilityStrategy::~HackingProbabilityStrategy(){
    // Pure destructors
};

std::unique_ptr<HackingProbabilityStrategy>
HackingProbabilityStrategy::build(json &config) {

  auto params = config.get<FrankenbachStrategy::Parameters>();
  return std::make_unique<FrankenbachStrategy>(params);
}

float FrankenbachStrategy::estimate(Experiment *experiment) {

  /// @todo replace this with boost comparison
  if ((params.base_hp - 0.) < 0.00001) {
    return 0.0;
  }

  // We have something in the middle now, so, we are calculating based on the
  // p-value we check for significance, if sig, then we return 0. else, then we
  // assign a value

  bool is_any_non_sig{false};
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {
    is_any_non_sig |= !experiment->dvs_[i].sig_;
  }

  if (!is_any_non_sig) {
    return 0.0;
  }

  // I have a feeling this is a very inefficient implementation
  int d = experiment->setup.nd();
  int g = experiment->setup.ng();

  arma::Row<float> dangers(g - d);

  dangers.imbue([&, i = d]() mutable {
    return this->border(experiment->dvs_[i++].se_effect_);
  });

  probabilities.resize(g - d);
  probabilities.imbue([&, i = d]() mutable {
    if (experiment->dvs_[i].effect_ > dangers[i - 1]) {

      float d_sig = experiment->dvs_[i].se_effect_ * 1.959964;
      arma::Row<float> danger_breaks =
          arma::linspace<arma::Row<float>>(dangers[i - 1], d_sig, 11);

      // If the hacking probability is 1, then everything in this range is
      // going to be hacked, a.k.a, hp = 1; Update: I think I had this wrong
      // previously, where I assign the probability to everything, while it
      // should only be assigned to those studies that are passing the effect
      // test in the first place
      if ((params.base_hp - 1.) < 0.00001) {
        return static_cast<float>(1);
      }

      return arma::as_scalar(hp_range.at(
          arma::max(find(danger_breaks < experiment->dvs_[i++].effect_))));
    }
    // else
    return static_cast<float>(0);
  });

  spdlog::trace("Chance of hacking: {}", arma::max(probabilities));

  // @todo Remember that you should consider some option here. At the moment,
  // I'm returning the maximum of all probabilities, but that's not necessarily
  // the best things to do, also, it works just fine in Frankenbach simulation
  // because they have only one one outcome anyway
  return arma::max(probabilities);
}
