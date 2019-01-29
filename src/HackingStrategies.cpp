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

Submission OutcomeSwitching::perform() {

    // std::cout << "Outcome Switching\n";

    long selectedOutcome = 0;
    
    if (_method == "min pvalue"){
        selectedOutcome = std::distance(this->experiment->pvalues.begin(), std::min_element(this->experiment->pvalues.begin(), this->experiment->pvalues.end()));

        // selectedOutcome = argMin(this->experiment->pvalues);
    }else if (_method == "max effect"){
        selectedOutcome = std::distance(this->experiment->effects.begin(), std::max_element(this->experiment->effects.begin(), this->experiment->effects.end()));
//         selectedOutcome = argMax(this->experiment->effects);
    }
    
    _selected_outcome_inx = selectedOutcome;
    return this->_create_submission_record(selectedOutcome);
}

Submission OutcomeSwitching::_create_submission_record(int inx) {
    Submission sub;
    sub.effect = experiment->effects[inx];
    sub.stat = experiment->statistics[inx];
    sub.pvalue = experiment->pvalues[inx];

    return sub;
}

Submission OptionalStopping::perform() {

    // std::cout << "Optional Stopping\n";

    if (!experiment->setup.isMultivariate){

    }else{
        auto newObs = experiment->dataStrategy->genNewObservationsForAllGroups(3);
        for (int i = 0; i < experiment->setup.ng; ++i) {
            experiment->measurements[i].insert(experiment->measurements[i].begin(),
                                                newObs[i].begin(),
                                                newObs[i].end());
        }
    }

    return hackedSubmission;
}
