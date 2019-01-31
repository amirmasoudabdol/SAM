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

Submission OutcomeSwitching::perform(Experiment experiment) {

    std::cout << "Outcome Switching\n";

    long selectedOutcome = 0;
    
    if (_method == "min pvalue"){
        selectedOutcome = std::distance(experiment.pvalues.begin(), std::min_element(experiment.pvalues.begin(), experiment.pvalues.end()));

        // selectedOutcome = argMin(experiment.pvalues);
    }else if (_method == "max effect"){
        selectedOutcome = std::distance(experiment.effects.begin(), std::max_element(experiment.effects.begin(), experiment.effects.end()));
//         selectedOutcome = argMax(this->experiment.effects);
    }
    
//    _selected_outcome_inx = selectedOutcome;
    return _create_submission_record(experiment, selectedOutcome);
}

Submission _create_submission_record(Experiment& experiment, int inx) {
    Submission sub;
    sub.effect = experiment.effects[inx];
    sub.stat = experiment.statistics[inx];
    sub.pvalue = experiment.pvalues[inx];

    return sub;
}

//Submission OutcomeSwitching::performOnCopy(Experiment expr) {
//    std::cout << "Outcome Switching on a Copy\n";
//
//    long selectedOutcome = 0;
//
//    if (_method == "min pvalue"){
//        selectedOutcome = std::distance(expr.pvalues.begin(), std::min_element(expr.pvalues.begin(), expr.pvalues.end()));
//
//        // selectedOutcome = argMin(expr.pvalues);
//    }else if (_method == "max effect"){
//        selectedOutcome = std::distance(expr.effects.begin(), std::max_element(expr.effects.begin(), expr.effects.end()));
//        //         selectedOutcome = argMax(expr.effects);
//    }
//
////        _selected_outcome_inx = selectedOutcome;
//    Submission sub;
//    sub.effect = expr.effects[selectedOutcome];
//    sub.stat = expr.statistics[selectedOutcome];
//    sub.pvalue = expr.pvalues[selectedOutcome];
////    return this->_create_submission_record(selectedOutcome);
////    return Submission();
//    return sub;
//}

Submission OptionalStopping::perform(Experiment experiment) {

     // std::cout << "Optional Stopping\n";

    if (experiment.setup.isMultivariate){

    }else{
        auto newObs = experiment.dataStrategy->genNewObservationsForAllGroups(_n_new_obs);
        for (int i = 0; i < experiment.setup.ng; ++i) {
            experiment.measurements[i].insert(experiment.measurements[i].begin(),
                                                newObs[i].begin(),
                                                newObs[i].end());
        }
    }

    experiment.calculateStatistics();
    experiment.calculateEffects();
    experiment.runTest();

    setSelectedOutcome(5);
    
//    std::cout << experiment.means << " :h\n";
    printVector(experiment.means); std::cout << " :h, means \n";
//    std::cout << experiment.measurements[0].size() << " :meas.size(), h\n";
    

    return _create_submission_record(experiment, 0);
}
