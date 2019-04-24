//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <stdexcept>

#include <SelectionStrategies.h>


SelectionStrategy::~SelectionStrategy() {
    // Pure deconstructor
}

SelectionStrategy *SelectionStrategy::build(json &config) {
    
    int selection_seed = rand();
    if (config["name"] == "SignificantSelection") {
        
        config["selection-seed"] = selection_seed;
        return new SignigicantSelection(config["alpha"], config["pub-bias"], config["side"], selection_seed);
        
    }else if(config["name"] == "RandomSelection") {
        
        config["selection-seed"] = selection_seed;
        return new RandomSelection(selection_seed);
        
    }else{
        throw std::invalid_argument("Unknown Selection Strategy.");
    }
}


/**
 Check if `p-value` of the Submission is less than the specified \f$\alpha\f$.
 If true, it will accept the submission, if not, a random number decide wheather
 the submission is going to be accepted or not. The drawn random number, \f$r\f$
 will be compared to `pub_bias` of the journal.

 @param s A reference to the Submission
 @return a boolean indicating whether the Submission is accepted or not.
 */
bool SignigicantSelection::review(Submission &s) {

    if (s.pvalue < alpha && (s.side == side || side == 0)){
        return true;
    }else if (mainRngStream->uniform() < pub_bias) {
        return true;
    }


    return false;
}

/**
 Draw a random number between \f$ r \in [0, 1] \f$, reject the submission if
 \f$ r < 0.5 \f$.
 
 @param s corresponding submission
 @return a boolean indicating whether the Submission is accpeted or not.
 */
bool RandomSelection::review(Submission &s) {
    if (mainRngStream->uniform() < 0.5) {
        return true;
    }else{
        return false;
    }
}
