//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <stdexcept>

#include "SelectionStrategy.h"

using namespace sam;

SelectionStrategy::~SelectionStrategy() {
    // Pure deconstructor
}

std::unique_ptr<SelectionStrategy> SelectionStrategy::build(json &selection_strategy_config) {
    
    int selection_seed = rand();
    if (selection_strategy_config["name"] == "SignificantSelection") {

        auto params = selection_strategy_config.get<SignificantSelection::Parameters>();
        params.seed =  selection_seed;
        return std::make_unique<SignificantSelection>(params);
        
    }else if(selection_strategy_config["name"] == "RandomSelection") {

        auto params = selection_strategy_config.get<RandomSelection::Parameters>();
        params.seed = selection_seed;
        return std::make_unique<RandomSelection>(params);
        
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
bool SignificantSelection::review(Submission &s) {

    // Only accepting +/- results if journal cares about it, side != 0
    if (s.side != params.side && params.side != 0){
        return false;
    }

    if (s.pvalue < params.alpha){
        return true;
    }else if (mainRngStream->uniform() < 1 - params.pub_bias) {
        return true;
    }else{
        return false;
    }
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
