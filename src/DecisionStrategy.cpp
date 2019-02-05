//
// Created by Amir Masoud Abdol on 2019-02-01.
//

#include "DecisionStrategy.h"

Submission DecisionStrategy::_select_Outcome(Experiment& experiment) {
    
    int selectedOutcome = preRegGroup;
    
//    std::cout << selectedOutcome << " selectedOutcome\n";
//    std::cout << experiment.means[0] << " :means, decision\n";
    
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
