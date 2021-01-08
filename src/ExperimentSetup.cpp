//
// Created by Amir Masoud Abdol on 2019-01-22.
//

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

  nobs_ = Parameter<int>(config["n_obs"], ng_);

  tsp_conf = config["test_strategy"];
  dsp_conf = config["data_strategy"];
  esp_conf = config["effect_strategy"];
}

void ExperimentSetup::randomizeTheParameters() { nobs_.randomize(); }

ExperimentSetupBuilder &ExperimentSetupBuilder::fromConfigFile(json &config) {

  setup.nc_ = config["n_conditions"];
  setup.nd_ = config["n_dep_vars"];
  setup.ng_ = setup.nc_ * setup.nd_;

  return *this;
}
