//
// Created by Amir Masoud Abdol on 2020-03-11
//

#include <iostream>
#include <string>

#include "DataStrategy.h"

using namespace sam;

DataStrategy::~DataStrategy() {
  // Pure destructors
}

std::unique_ptr<DataStrategy> DataStrategy::build(json &data_strategy_config) {
  
  spdlog::debug("Building a Data Strategy");

  if (data_strategy_config["name"] == "LinearModel") {
    auto params = data_strategy_config.get<LinearModelStrategy::Parameters>();
    return std::make_unique<LinearModelStrategy>(params);

  } else if (data_strategy_config["name"] == "LatentModel") {
    return std::make_unique<LatentDataStrategy>();

  } else if (data_strategy_config["name"] == "GradedResponseModel") {
    auto params = data_strategy_config.get<GRMDataStrategy::Parameters>();
    return std::make_unique<GRMDataStrategy>(params);

  } else {
    throw std::invalid_argument("Unknown Data Strategy.");
  }
}
