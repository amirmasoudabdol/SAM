//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <stdexcept>
#include <tuple>

#include "ExperimentSetup.h"

using namespace sam;

ExperimentSetupBuilder ExperimentSetup::create() {
  return ExperimentSetupBuilder();
}

ExperimentSetup::ExperimentSetup(json &config) {

  nc_ = config["n_conditions"];
  nd_ = config["n_dep_vars"];
  ng_ = nc_ * nd_;
  
  n_reps_ = config["n_reps"];

  auto nobs_t = get_expr_setup_params<int>(config["n_obs"], ng_);
  nobs_ = nobs_t;

  tsp_conf = config["test_strategy"];
  dsp_conf = config["data_strategy"];
  esp_conf = config["effect_strategy"];
}

void ExperimentSetup::randomize_parameters() {

  //    if (params_dist["n_obs"]){
  //        fill_vector<int>(nobs_, nobs_.size(),
  //        Random::get(params_dist["n_obs"]));
  //    }
}

ExperimentSetupBuilder &ExperimentSetupBuilder::fromConfigFile(json &config) {

  setup.nc_ = config["n_conditions"];
  setup.nd_ = config["n_dep_vars"];
  setup.ng_ = setup.nc_ * setup.nd_;

  return *this;
}
