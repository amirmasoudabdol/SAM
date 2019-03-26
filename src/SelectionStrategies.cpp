//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <SelectionStrategies.h>

SelectionStrategy::~SelectionStrategy() {
    // Pure deconstructor
}





/**
 Review the given submission based on its _p_-value and its effect side.
 
 @param s A submission record to be reviewed
 @return A boolean value indicating the acceptance status
 */
bool SignigicantSelection::review(const Submission &s) {
    if (s.pvalue < _alpha && (s.side == _side || _side == 0)){
        return true;
    }else if (mainRngStream->uniform() < _pub_bias) {
        return true;
    }

    return false;
}


SelectionStrategy *SelectionStrategy::build(json &config) {
//    if (config["--selection-strategy"] == "SignificantSelection") {
        int selectionSeed = rand();
        config["--selection-seed"] = selectionSeed;
        return new SignigicantSelection(config["--pub-bias"], config["--alpha"], config["--side"], selectionSeed);
//    }
}
