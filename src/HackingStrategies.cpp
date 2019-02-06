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

    long selectedOutcome = 0;
    
    if (_method == "MinPvalue"){
        selectedOutcome = std::distance(experiment->pvalues.begin(), std::min_element(experiment->pvalues.begin(), experiment->pvalues.end()));

    }else if (_method == "MaxEffect"){
        selectedOutcome = std::distance(experiment->effects.begin(), std::max_element(experiment->effects.begin(), experiment->effects.end()));

    }
    
}

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
    
    // This can also ask for the verdict and if it's not fine,
    // go for more trials.
    
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
