//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <HackingStrategies.h>
#include <Experiment.h>
#include <Utilities.h>
#include <DecisionStrategy.h>

#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>

std::ostream& operator<<(std::ostream& os, HackingMethod m)
{
    switch(m)
    {
        case _OutcomeSwitching   : os << "Outcome Switching";    break;
        case _OptionalStopping   : os << "Optional Stopping"; break;
        case _OutlierRemoval     : os << "Outlier Removal";  break;
        default    : os.setstate(std::ios_base::failbit);
    }
    return os;
}

/**
 This has been deprecated. The outcome switching is now implemeted as part of DecisionStrategy
 where the researcher specifies how he'd approach selecting an outcome. This is determinded by
 ResearcherPreference.
 */
void OutcomeSwitching::perform(Experiment* experiment, DecisionStrategy* decisionStrategy) {

    long selectedOutcome = 0;
    
    if (_method == "MinPvalue"){
        selectedOutcome = std::distance(experiment->pvalues.begin(), std::min_element(experiment->pvalues.begin(), experiment->pvalues.end()));

    }else if (_method == "MaxEffect"){
        selectedOutcome = std::distance(experiment->effects.begin(), std::max_element(experiment->effects.begin(), experiment->effects.end()));

    }
    
}

/**
 \brief Implementation of optional stopping.
 
 This will use two parameters set at construction of the OptionalStopping class, `_n_trials` and `_n_new_obs`
 for every trial, the routine will add `_n_new_obs` to all groups, recalculate the statistics, and run the test. It will then select an outcome based on researcher's preference and check it's significance. If
 the results is significant, it'll not make a new attempt to add more data, and will return to the hack() routine.
 */
void OptionalStopping::perform(Experiment* experiment, DecisionStrategy* decisionStrategy) {
    
    Submission tmpSub;
    
    // If `_n_new_obs` is not specified
    if (_n_new_obs == 0){
        _n_new_obs = experiment->setup.nobs / 3;
    }
    
    for (int t = 0; t < _n_trials; t++) {
        
//        if (experiment->setup.isMultivariate){
//            // I can move this check to the dataStrategy and basically set it as Static when I'm initiating the class!
//        }else{
            auto newObs = experiment->dataStrategy->genNewObservationsForAllGroups(experiment, _n_new_obs);
            for (int i = 0; i < experiment->setup.ng; ++i) {
                experiment->measurements[i].insert(experiment->measurements[i].begin(),
                                                    newObs[i].begin(),
                                                    newObs[i].end());
            }
//        }

        // Recalculate the experiment
        experiment->calculateStatistics();
        experiment->calculateEffects();
        experiment->runTest();
        
        tmpSub = decisionStrategy->_select_Outcome(*experiment);
        
        if (tmpSub.isSig()) break;
        
    }
    
}



/**
 \brief Implementation of Outliers Removal based on an item's distance from the \f$\mu\f$.
 
 The `_sd_multiplier`, \f$d\f$ is set at the construction, and it'll be used to check whether a measurement
 should be removed or not. Any item satisfying the \f$ v > |\mu - d * \sigma| \f$ will be removed from the
 dataset.
 */
void SDOutlierRemoval::perform(Experiment* experiment, DecisionStrategy* decisionStrategy){
    
    int gi = 0;     // Only to access the means, vars
    for (auto &g : experiment->measurements) {
        
        g.erase(std::remove_if(g.begin(),
                               g.end(),
                               [this, experiment, gi](double v){ return (v < experiment->means[gi] - this->_sd_multiplier * sqrt(experiment->vars[gi]))
                                                                        ||
                                                                        (v > experiment->means[gi] + this->_sd_multiplier * sqrt(experiment->vars[gi])); }),
                g.end()
                );
        
        gi++;
    }
    
    // Recalculate the experiment
    experiment->calculateStatistics();
    experiment->calculateEffects();
    experiment->runTest();
}


HackingStrategy *HackingStrategy::buildHackingMethod(json h_params) {
    std::string type = h_params["type"];
    if (type == "OptionalStopping"){
        return new OptionalStopping(h_params["size"],           h_params["attempts"]);
    }else if (type == "SDOutlierRemoval") {
        return new SDOutlierRemoval(h_params["sd_multiplier"]);
    }else if (type == "GroupPooling") {
        return new GroupPooling("first");
    }else{
        return new OutcomeSwitching("none");
    }
    
}


void GroupPooling::perform(Experiment *experiment, DecisionStrategy *decisionStrategy) {
    
}
