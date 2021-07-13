//===-- ExperimentSetup.cpp - Experiment Setup Definition ----------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-01-22.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the definition of ExperimentSetup methods.
///
//===----------------------------------------------------------------------===//

#include "ExperimentSetup.h"

using namespace sam;

/// This set and calculates design parameters of the experiment. It also archives
/// the definition of data, test, and effect strategies. These archives may later
/// be retrieved by the Journal during the review process.
///
/// @param config A JSON configuration object
ExperimentSetup::ExperimentSetup(json &config) {

  updateDesignParameters(config["n_conditions"], config["n_dep_vars"]);

  if (config.contains("n_reps")) {
    n_reps_ = config["n_reps"];
  }

  nobs_ = Parameter<int>(config["n_obs"], ng_);

  dsp_conf = config["data_strategy"];
  tsp_conf = config["test_strategy"];
  esp_conf = config["effect_strategy"];
}

/// This randomizes all those parameters of the ExperimentSetup that are defined
/// by a distribution
void ExperimentSetup::randomize() {
  nobs_();
}


// ------------------------ //
// Experiment Setup Builder //
// ------------------------ //

ExperimentSetupBuilder ExperimentSetup::create() {
  return ExperimentSetupBuilder();
}

ExperimentSetupBuilder &ExperimentSetupBuilder::fromConfigFile(json &config) {

  setup.nc_ = config["n_conditions"];
  setup.nd_ = config["n_dep_vars"];
  setup.ng_ = setup.nc_ * setup.nd_;

  return *this;
}
