//===-- HackingStrategy.cpp - Hacking Strategy Abstract Class Imp  --------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-01-25.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the implementation of the hacking strategy abstract
/// class, e.g., the abstract builder, and pure constructor implementation.
///
//===----------------------------------------------------------------------===//

#include "HackingStrategy.h"

using namespace sam;

HackingStrategy::~HackingStrategy(){
    // Pure destructors
}

HackingStrategy::HackingStrategy() {
  lua.open_libraries();

  lua.new_usertype<DependentVariable>("DependentVariable",
   "id", &DependentVariable::id_,
   "nobs", &DependentVariable::nobs_,
   "mean", &DependentVariable::mean_,
   "pvalue", &DependentVariable::pvalue_,
   "effect", &DependentVariable::effect_,
   "sig", &DependentVariable::sig_,
   "hacked", &DependentVariable::is_hacked_,
   "candidate", &DependentVariable::is_candidate_
   );

  lua.new_usertype<Submission>("Submission",
      "id", sol::property([](Submission &s) { return s.dv_.id_; }),
      "nobs", sol::property([](Submission &s) { return s.dv_.nobs_; }),
      "mean", sol::property([](Submission &s) { return s.dv_.mean_; }),
      "pvalue", sol::property([](Submission &s) { return s.dv_.pvalue_; }),
      "effect", sol::property([](Submission &s) { return s.dv_.effect_; }),
      "sig", sol::property([](Submission &s) { return s.dv_.sig_; }),
      "hacked", sol::property([](Submission &s) { return s.dv_.is_hacked_; }),
      "candidate", sol::property([](Submission &s) { return s.dv_.is_candidate_; }));
}

///
/// A Factory method for building hacking strategies
///
/// @param      config  A JSON object defining a hacking strategy, and its
///                     parameters
/// @return     Pointer to a HackingStrategy
///
std::unique_ptr<HackingStrategy>
HackingStrategy::build(json &hacking_strategy_config) {
  
  spdlog::debug("Building a Hacking Strategy");

  if (hacking_strategy_config["name"] == "OptionalStopping") {

    auto params = hacking_strategy_config.get<OptionalStopping::Parameters>();
    return std::make_unique<OptionalStopping>(params);

  } else if (hacking_strategy_config["name"] == "OutliersRemoval") {

    auto params = hacking_strategy_config.get<OutliersRemoval::Parameters>();
    return std::make_unique<OutliersRemoval>(params);

  } else if (hacking_strategy_config["name"] == "GroupPooling") {

    auto params = hacking_strategy_config.get<GroupPooling::Parameters>();
    return std::make_unique<GroupPooling>(params);

  } else if (hacking_strategy_config["name"] == "ConditionDropping") {

    auto params = hacking_strategy_config.get<ConditionDropping::Parameters>();
    return std::make_unique<ConditionDropping>(params);

  } else if (hacking_strategy_config["name"] == "SubjectiveOutlierRemoval") {

    auto params =
        hacking_strategy_config.get<SubjectiveOutlierRemoval::Parameters>();
    return std::make_unique<SubjectiveOutlierRemoval>(params);

  } else if (hacking_strategy_config["name"] == "QuestionableRounding") {
    
    auto params =
    hacking_strategy_config.get<QuestionableRounding::Parameters>();
    return std::make_unique<QuestionableRounding>(params);
    
  } else if (hacking_strategy_config["name"] == "PeekingOutliersRemoval") {
    
    auto params =
    hacking_strategy_config.get<PeekingOutliersRemoval::Parameters>();
    return std::make_unique<PeekingOutliersRemoval>(params);
    
  } else if (hacking_strategy_config["name"] == "FalsifyingData") {
    
    auto params =
    hacking_strategy_config.get<FalsifyingData::Parameters>();
    return std::make_unique<FalsifyingData>(params);
    
  } else if (hacking_strategy_config["name"] == "FabricatingData") {
    
    auto params =
    hacking_strategy_config.get<FabricatingData::Parameters>();
    return std::make_unique<FabricatingData>(params);
    
  } else if (hacking_strategy_config["name"] == "StoppingDataCollection") {
    
    auto params =
    hacking_strategy_config.get<StoppingDataCollection::Parameters>();
    return std::make_unique<StoppingDataCollection>(params);
    
  } else if (hacking_strategy_config["name"] == "NoHack") {

    return std::make_unique<NoHack>();

  } else {
    throw std::invalid_argument("Unknown Hacking Strategies.");
  }
  
}
