//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <HackingStrategies.h>
#include <Experiment.h>
#include <Utilities.h>
#include <DecisionStrategy.h>

#include <vector>
#include <armadillo>
#include <iostream>
#include <algorithm>
#include <numeric>

std::ostream& operator<<(std::ostream& os, HackingMethod m)
{
    switch(m)
    {
        case HackingMethod::OutcomeSwitching   : os << "Outcome Switching";    break;
        case HackingMethod::OptionalStopping   : os << "Optional Stopping"; break;
        case HackingMethod::OutlierRemoval     : os << "Outlier Removal";  break;
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
    
    if (_method == "Min P-value"){
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
    
    // FIXME: Commented during the migration
    // IMPROVE ME!
    for (int t = 0; t < _n_trials; t++) {

            auto newObs = experiment->dataStrategy->genNewObservationsForAllGroups(experiment, _n_new_obs);
            for (int i = 0; i < experiment->setup.ng; ++i) {
//                experiment->measurements[i].insert(experiment->measurements[i].begin(),
//                                                    newObs[i].begin(),
//                                                    newObs[i].end());
                // IMPROVE ME: Maybe I want to use join instead.
                experiment->measurements[i].insert_cols(experiment->measurements.size(), newObs[i]);
            }

        // Recalculate the experiment
        experiment->calculateStatistics();
        experiment->calculateEffects();
        experiment->runTest();

        tmpSub = decisionStrategy->_select_Outcome(*experiment);

        if (tmpSub.isSig())
            break;

    }

}



/**
 \brief Implementation of Outliers Removal based on an item's distance from the \f$\mu\f$.
 
 The `_sd_multiplier`, \f$d\f$ is set at the construction, and it'll be used to check whether a measurement
 should be removed or not. Any item satisfying the \f$ v > |\mu - d * \sigma| \f$ will be removed from the
 dataset.
 */
void SDOutlierRemoval::perform(Experiment* experiment, DecisionStrategy* decisionStrategy){
    
    // FIXME: Commented during the migration
    int gi = 0;     // Only to access the means, vars
    for (auto &g : experiment->measurements) {

//        g.erase(std::remove_if(g.begin(),
//                               g.end(),
//                               [this, experiment, gi](double v){ return (v < experiment->means[gi] - this->_sd_multiplier * sqrt(experiment->vars[gi]))
//                                                                        ||
//                                                                        (v > experiment->means[gi] + this->_sd_multiplier * sqrt(experiment->vars[gi])); }),
//                g.end()
//                );
        
        // IMPROVE ME!
        arma::uvec inx = arma::find(
                              (g < experiment->means[gi] - this->_sd_multiplier * sqrt(experiment->vars[gi]))
                              ||
                              (g > experiment->means[gi] + this->_sd_multiplier * sqrt(experiment->vars[gi]))
                              );
        for (int i = 0 ; i < inx.size(); i++) {
            g.shed_col(i);
        }

        gi++;
    }
    
    // Recalculate the experiment
    experiment->calculateStatistics();
    experiment->calculateEffects();
    experiment->runTest();
    
    // Extreme
//    removeOutliers(1, _max_attempts, _max_attempts, experiment);
    
    // Recursive
//    removeOutliers(_num, _max_attempts, _max_attempts, experiment);
    
    // Recursive Attempts
//    removeOutliers(_num, _attempts, _max_attempts, experiment);
    
}


// FIXME: Commented during the migration
//void SDOutlierRemoval::removeOutliers(int n, int t, int m, Experiment* experiment) {
//    for (auto &d : _multipliers){
//        for (int i = 0, gi = 0; i < t && i < m; i++) {
//            for (auto &g : experiment->measurements) {
//
//                auto outliers = std::remove_if(g.begin(),
//                                               g.end(),
//                                               [experiment, gi, d](double v){
//                                                   return
//                                                   (v < experiment->means[gi] - d * sqrt(experiment->vars[gi]))
//                                                   ||
//                                                   (v > experiment->means[gi] + d * sqrt(experiment->vars[gi]));
//                                               }
//                                               );
//                if (outliers != g.end())
//                    break;
//
//                g.erase(outliers,
//                        g.end()
//                        );
//
//                gi++;
//            }
//
//        }
//        // Update everything and ask for verdict
//    }
//}



/**
 A Factory method for building hacking strategies
 
 \sa README.md

 @param config A JSON object defining a hacking strategy, and its parameters
 @return Pointer to a HackingStrategy
 */
HackingStrategy *HackingStrategy::buildHackingMethod(json& config) {
    std::string type = config["type"];
    if (type == "Optional Stopping"){
        return new OptionalStopping(config["size"], config["attempts"]);
    }else if (type == "SD Outlier Removal") {
        return new SDOutlierRemoval(config["sd_multiplier"]);
    }else if (type == "Group Pooling") {
        return new GroupPooling("first");
    }else{
        // This is not ok, this final else should be resolved somehow
        return new OutcomeSwitching("none");
    }
    
}


void GroupPooling::perform(Experiment *experiment, DecisionStrategy *decisionStrategy) {
    
}
