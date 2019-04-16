//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <SelectionStrategies.h>

SelectionStrategy::~SelectionStrategy() {
    // Pure deconstructor
}


bool SignigicantSelection::review(const Submission &s) {
    if (s.pvalue < alpha && (s.side == side || side == 0)){
        return true;
    }else if (mainRngStream->uniform() < pub_bias) {
        return true;
    }

    return false;
}


SelectionStrategy *SelectionStrategy::build(json &config) {
//    if (config["selection-strategy"] == "SignificantSelection") {
        int selectionSeed = rand();
        config["selection-seed"] = selectionSeed;
        return new SignigicantSelection(config["pub-bias"], config["alpha"], config["side"], selectionSeed);
//    }
}
