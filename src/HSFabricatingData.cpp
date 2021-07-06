//===-- HSFabricatingData.cpp - Fabricating Data Implementation
//---------------------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2020-08-28.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the implementation of the Fabricating Data hacking
/// strategy and its related subroutines.
///
//===----------------------------------------------------------------------===//

#include "DataStrategy.h"
#include "HackingStrategy.h"

using namespace sam;

void FabricatingData::perform(Experiment *experiment) {

  spdlog::debug("Falsifying Data: ");

  bool res{true};

  for (int t = 0; t < params.n_attempts && res; ++t) {

    if (params.approach == "generating") {
      res = this->generate(experiment, params.num);
    } else if (params.approach == "duplicating") {
      res = this->duplicate(experiment, params.num);
    } else {
      spdlog::critical("Falsifying approach is not recognized!");
      exit(1);
    }

    experiment->recalculateEverything();

    spdlog::trace("{}", *experiment);

    if (!params.stopping_cond_defs.empty()) {
      if (stopping_condition(experiment)) {
        spdlog::trace("⚠️ Stopping the hacking procedure, stopping condition "
                      "has been met!");
        return;
      }
    }
  }
}

///
/// The algorithm generates `n` new observations based on the defined `dist`
/// distribution.
///
/// @note       If you would like the new observations to be generated based on
/// the true population, you should use the OptionalStopping method instead.
///
/// @param      experiment  The pointer to the experiment
/// @param[in]  n           Number of new items to be generated
///
/// @return     { description_of_the_return_value }
///
bool FabricatingData::generate(Experiment *experiment, const int n) {

  spdlog::debug(" → Generating fake data...");

  int begin{0};
  int end{0};
  std::tie(begin, end) = getTargetBounds(experiment, params.target);

  for (int i = begin; i < end; ++i) {

    static arma::Row<float> new_observations(n, arma::fill::zeros);
    new_observations.imbue([&]() { return Random::get(params.dist.value()); });

    experiment->dvs_[i].addNewMeasurements(new_observations);
  }

  return true;
}

///
/// The algorithm simply randomly selects `n` observations, duplicates them and
/// add them back to the same dependent variable.
///
/// @param      experiment  The points to the experiment
/// @param[in]  n           Indicates the number of datapoints to be duplicated
///
/// @return     Return `true` if everything goes well
///
bool FabricatingData::duplicate(Experiment *experiment, const int n) {

  spdlog::debug(" → Duplicating some data...");

  int begin{0};
  int end{0};
  std::tie(begin, end) = getTargetBounds(experiment, params.target);

  for (int i = begin; i < end; ++i) {

    arma::Row<float> row = arma::shuffle((*experiment)[i].measurements());
    arma::Row<float> copy_candidates = row.head(n);

    experiment->dvs_[i].addNewMeasurements(copy_candidates);
  }

  return true;
}

///
/// The algorithm copies `n` observations, adds some noise to them and add them
/// back to the dependent variable.
///
/// @param      experiment  The experiment
/// @param[in]  n           Indicates the number of items to be copied and
/// perturbed
///
/// @return     Return `true` if everything goes well.
///
bool FabricatingData::manipulate(Experiment *experiment, int n) {

  spdlog::debug(" → Manipulating some data...");

  int begin{0};
  int end{0};
  std::tie(begin, end) = getTargetBounds(experiment, params.target);

  for (int i = begin; i < end; ++i) {

    arma::Row<float> row = arma::shuffle((*experiment)[i].measurements());
    arma::Row<float> copy_candidates = row.head(n);

    static arma::Row<float> noise(n, arma::fill::zeros);
    noise.imbue([&]() { return Random::get(params.noise.value()); });

    experiment->dvs_[i].addNewMeasurements(copy_candidates + noise);
  }

  return true;
}
