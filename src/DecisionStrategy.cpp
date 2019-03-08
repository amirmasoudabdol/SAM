//
// Created by Amir Masoud Abdol on 2019-02-01.
//

#include "DecisionStrategy.h"


std::ostream& operator<<(std::ostream& os, ResearcherPreference rp)
{
    switch(rp)
    {
        case ResearcherPreference::PreRegisteredOutcome : os << "PreRegistered Outcome";  break;
        case ResearcherPreference::MinSigPvalue : os << "Minimum Significant Pvalue"; break;
        case ResearcherPreference::MinPvalue : os << "Minimum Pvalue";  break;
        case ResearcherPreference::MaxSigEffect : os << "Maximum Significant Effect"; break;
        case ResearcherPreference::MaxEffect : os << "Maximum Effect"; break;
        case ResearcherPreference::MinPvalueMaxEffect : os << "Maximum Effect with Min Pvalue"; break;
        default    : os.setstate(std::ios_base::failbit);
    }
    return os;
}

Submission DecisionStrategy::_select_Outcome(Experiment& experiment) {
    
    int selectedOutcome = preRegGroup;
    
    switch (selectionPref) {
        case ResearcherPreference::PreRegisteredOutcome:
            selectedOutcome = preRegGroup;
            break;
            
        case ResearcherPreference::MinSigPvalue:
            
            break;
            
        case ResearcherPreference::MinPvalue:
            selectedOutcome = std::distance(experiment.pvalues.begin(), std::min_element(experiment.pvalues.begin(), experiment.pvalues.end()));
            break;
            
        case ResearcherPreference::MaxSigEffect:
            
            break;
            
        case ResearcherPreference::MaxEffect:
            selectedOutcome = std::distance(experiment.effects.begin(), std::max_element(experiment.effects.begin(), experiment.effects.end()));
            break;
            
        case ResearcherPreference::MinPvalueMaxEffect:
            
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
    return new ImpatientDecisionMaker(0,
                                      config["alpha"],
                                      stringToResearcherPreference.find(config["preference"])->second);
//    }
}

