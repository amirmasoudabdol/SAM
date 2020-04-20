//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <DecisionStrategy.h>
#include <Experiment.h>
#include <HackingStrategy.h>

#include <numeric>
#include <stdexcept>

using namespace sam;

HackingStrategy::~HackingStrategy(){
    // Pure deconstructor
};

HackingStrategy::HackingStrategy() {
  lua.open_libraries();

  lua.new_usertype<GroupData>("GroupData", "id", &GroupData::id_, "nobs",
                              &GroupData::nobs_, "pvalue", &GroupData::pvalue_,
                              "effect", &GroupData::effect_, "sig",
                              &GroupData::sig_);

  lua.new_usertype<Submission>(
      "Submission", "id",
      sol::property([](Submission &s) { return s.group_.id_; }), "nobs",
      sol::property([](Submission &s) { return s.group_.nobs_; }), "mean",
      sol::property([](Submission &s) { return s.group_.mean_; }), "pvalue",
      sol::property([](Submission &s) { return s.group_.pvalue_; }), "effect",
      sol::property([](Submission &s) { return s.group_.effect_; }), "sig",
      sol::property([](Submission &s) { return s.group_.sig_; }));
}

///
/// A Factory method for building hacking strategies
///
/// \param      config  A JSON object defining a hacking strategy, and its
///                     parameters
/// \return     Pointer to a HackingStrategy
///
std::unique_ptr<HackingStrategy>
HackingStrategy::build(json &hacking_strategy_config) {

  if (hacking_strategy_config["_name"] == "OptionalStopping") {

    auto params = hacking_strategy_config.get<OptionalStopping::Parameters>();
    return std::make_unique<OptionalStopping>(params);

  } else if (hacking_strategy_config["_name"] == "OutliersRemoval") {

    auto params = hacking_strategy_config.get<OutliersRemoval::Parameters>();
    return std::make_unique<OutliersRemoval>(params);

  } else if (hacking_strategy_config["_name"] == "GroupPooling") {

    auto params = hacking_strategy_config.get<GroupPooling::Parameters>();
    return std::make_unique<GroupPooling>(params);

  } else if (hacking_strategy_config["_name"] == "ConditionDropping") {

    auto params = hacking_strategy_config.get<ConditionDropping::Parameters>();
    return std::make_unique<ConditionDropping>(params);

  } else if (hacking_strategy_config["_name"] == "SubjectiveOutlierRemoval") {

    auto params =
        hacking_strategy_config.get<SubjectiveOutlierRemoval::Parameters>();
    return std::make_unique<SubjectiveOutlierRemoval>(params);

  } else if (hacking_strategy_config["_name"] == "NoHack") {

    return std::make_unique<NoHack>();

  } else {
    throw std::invalid_argument("Unknown Hacking Strategies.");
  }
}

