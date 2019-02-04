//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <HackingStrategies.h>
#include <Experiment.h>
#include <Utilities.h>

#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>

void OutcomeSwitching::perform(Experiment* experiment) {

//    std::cout << "Outcome Switching\n";

    long selectedOutcome = 0;
    
    if (_method == "min pvalue"){
        selectedOutcome = std::distance(experiment->pvalues.begin(), std::min_element(experiment->pvalues.begin(), experiment->pvalues.end()));

        // selectedOutcome = argMin(experiment->pvalues);
    }else if (_method == "max effect"){
        selectedOutcome = std::distance(experiment->effects.begin(), std::max_element(experiment->effects.begin(), experiment->effects.end()));
//         selectedOutcome = argMax(this->experiment->effects);
    }
    
//    _selected_outcome_inx = selectedOutcome;
//    return _create_submission_record(*experiment, selectedOutcome);
//    return Submission(*experiment, selectedOutcome);
}

//Submission _create_submission_record(Experiment& experiment, int inx) {
//    Submission sub;
//    sub.effect = experiment.effects[inx];
//    sub.statistic = experiment.statistics[inx];
//    sub.pvalue = experiment.pvalues[inx];
//
//    return sub;
//}

void OptionalStopping::perform(Experiment* experiment) {

     // std::cout << "Optional Stopping\n";

    // TODO: Implemented the multi_trial
    
    if (experiment->setup.isMultivariate){

    }else{
        auto newObs = experiment->dataStrategy->genNewObservationsForAllGroups(_n_new_obs);
        for (int i = 0; i < experiment->setup.ng; ++i) {
            experiment->measurements[i].insert(experiment->measurements[i].begin(),
                                                newObs[i].begin(),
                                                newObs[i].end());
        }
    }

    experiment->calculateStatistics();
    experiment->calculateEffects();
    experiment->runTest();
    
    // TODO: this needs to change, it needs to pick and select instead of 0;
//    return _create_submission_record(*experiment, 0);
//    return Submission(*experiment, 0);
}


//Submission OutlierRemoval::perform(Experiment* experiment) {
//    
//    
//    
//    
//    // TODO: this needs to change, it needs to pick and select instead of 0;
//    return _create_submission_record(*experiment, 0);
//}
