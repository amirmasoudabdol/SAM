//
// Created by Amir Masoud Abdol on 2020-03-11
//

#include <iostream>
#include <string>

#include "DataStrategy.h"

using namespace sam;

DataStrategy::~DataStrategy() {
  // Pure deconstructor
}

std::unique_ptr<DataStrategy> DataStrategy::build(json &data_strategy_config) {

  if (data_strategy_config["_name"] == "LinearModel") {
    auto params = data_strategy_config.get<LinearModelStrategy::Parameters>();
    return std::make_unique<LinearModelStrategy>(params);

  } else if (data_strategy_config["_name"] == "LatentModel") {
    return std::make_unique<LatentDataStrategy>();

  } else if (data_strategy_config["_name"] == "GradedResponseModel") {
    auto params = data_strategy_config.get<GRMDataStrategy::Parameters>();
    return std::make_unique<GRMDataStrategy>(params);

  } else {
    throw std::invalid_argument("Unknown Data Strategy.");
  }
}
