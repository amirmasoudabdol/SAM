//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <DecisionStrategy.h>
#include <Experiment.h>
#include <HackingStrategy.h>

#include <numeric>
#include <stdexcept>

using namespace sam;

HackingStrategy::~HackingStrategy() {
    // Pure deconstructor
};

/**
 A Factory method for building hacking strategies

 @param config A JSON object defining a hacking strategy, and its parameters
 @return Pointer to a HackingStrategy
 */
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

std::unique_ptr<HackingStrategy> HackingStrategy::build(HackingMethod method) {
    switch (method) {

        case HackingMethod::NoHack:
            return std::make_unique<NoHack>();
            break;
        case HackingMethod::OptionalStopping:
            return std::make_unique<OptionalStopping>();
            break;
        case HackingMethod::OutliersRemoval:
            return std::make_unique<OutliersRemoval>();
            break;
        case HackingMethod::GroupPooling:
            return std::make_unique<GroupPooling>();
            break;
        case HackingMethod::ConditionDropping:
            return std::make_unique<ConditionDropping>();
            break;
        default:
            return std::make_unique<NoHack>();
            break;
    }
}
