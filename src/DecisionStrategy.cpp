//
// Created by Amir Masoud Abdol on 2019-02-01.
//

#include "DecisionStrategy.h"


std::ostream& operator<<(std::ostream& os, ResearcherPreference rp)
{
    switch(rp)
    {
        case PreRegisteredOutcome : os << "PreRegistered Outcome";  break;
        case MinSigPvalue : os << "Minimum Significant Pvalue"; break;
        case MinPvalue : os << "Minimum Pvalue";  break;
        case MaxSigEffect : os << "Maximum Significant Effect"; break;
        case MaxEffect : os << "Maximum Effect"; break;
        case MinPvalueMaxEffect : os << "Maximum Effect with Min Pvalue"; break;
        default    : os.setstate(std::ios_base::failbit);
    }
    return os;
}

Submission DecisionStrategy::_select_Outcome(Experiment& experiment) {
    
    int selectedOutcome = preRegGroup;
    
    switch (selectionPref) {
        case PreRegisteredOutcome:
            selectedOutcome = preRegGroup;
            break;
            
        case MinSigPvalue:
            
            break;
            
        case MinPvalue:
            selectedOutcome = std::distance(experiment.pvalues.begin(), std::min_element(experiment.pvalues.begin(), experiment.pvalues.end()));
            break;
            
        case MaxSigEffect:
            
            break;
            
        case MaxEffect:
            selectedOutcome = std::distance(experiment.effects.begin(), std::max_element(experiment.effects.begin(), experiment.effects.end()));
            break;
            
        case MinPvalueMaxEffect:
            
            break;
            
    }
    
    return Submission(experiment, selectedOutcome);
}