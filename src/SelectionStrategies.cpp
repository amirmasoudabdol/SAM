//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <stdexcept>

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
    
    int selection_seed = rand();
    if (config["selection-strategy"] == "SignificantSelection") {
        
        config["selection-seed"] = selection_seed;
        return new SignigicantSelection(config["pub-bias"], config["alpha"], config["side"], selection_seed);
    
    }else if(config["selection-strategy"] == "RandomSelection") {
        
        config["selection-seed"] = selection_seed;
        return new RandomSelection(selection_seed);
    
    }else{
        throw std::invalid_argument("Unknown Selection Strategy.");
    }
}


bool RandomSelection::review(const Submission &s) {
    if (mainRngStream->uniform() < 0.5) {
        return true;
    }else{
        return false;
    }
}
