//
// Created by Amir Masoud Abdol on 2019-02-01.
//

#include "DecisionStrategy.h"

/**
 * Pure deconstructor of DecisionStrategy.
 * This is necessary for proper deconstruction of derived
 * classes.
 */
DecisionStrategy::~DecisionStrategy() {
    // pure deconstructor
};

std::ostream& operator<<(std::ostream& os, DecisionPreference dp)
{
    switch(dp)
    {
        case DecisionPreference::PreRegisteredOutcome:
            os << "PreRegistered Outcome";
            break;
        case DecisionPreference::MinSigPvalue:
            os << "Minimum Significant Pvalue";
            break;
        case DecisionPreference::MinPvalue:
            os << "Minimum Pvalue";
            break;
        case DecisionPreference::MaxSigEffect:
            os << "Maximum Significant Effect";
            break;
        case DecisionPreference::MaxEffect:
            os << "Maximum Effect";
            break;
        case DecisionPreference::MinPvalueMaxEffect:
            os << "Maximum Effect with Min Pvalue";
            break;
        default:
            // CHECKME: What's this?
            os.setstate(std::ios_base::failbit);
    }
    return os;
}

Submission DecisionStrategy::_select_Outcome(Experiment& experiment) {
    
    int selectedOutcome = preRegGroup;
    
    switch (selectionPref) {
        case DecisionPreference::PreRegisteredOutcome:
            selectedOutcome = preRegGroup;
            break;
            
        case DecisionPreference::MinSigPvalue:
            
            break;
            
        case DecisionPreference::MinPvalue:
            selectedOutcome = std::distance(experiment.pvalues.begin(), std::min_element(experiment.pvalues.begin(), experiment.pvalues.end()));
            break;
            
        case DecisionPreference::MaxSigEffect:
            
            break;
            
        case DecisionPreference::MaxEffect:
            selectedOutcome = std::distance(experiment.effects.begin(), std::max_element(experiment.effects.begin(), experiment.effects.end()));
            break;
            
        case DecisionPreference::MinPvalueMaxEffect:
            
            break;
            
        default:
            selectedOutcome = 0;
            break;
    }
    
    Submission tmpSub(experiment, selectedOutcome);
    
    return tmpSub;
}

DecisionStrategy *DecisionStrategy::buildDecisionStrategy(json &config) {
    // TODO: Expand
//    if (config["type"] == "Impatient Decision Maker"){
    return new ImpatientDecisionMaker(stringToResearcherPreference.find(config["preference"])->second);
//    }
}

