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
#include <cmath>
#include <stdexcept>

#include "permutation.h"

#include "main.h"

HackingStrategy::~HackingStrategy() {
    // Pure deconstructor
};

std::ostream& operator<<(std::ostream& os, HackingMethod m)
{
    switch(m)
    {
        case HackingMethod::OptionalStopping:
            os << "Optional Stopping";
            break;
        case HackingMethod::OutlierRemoval:
            os << "Outlier Removal";
            break;
        case HackingMethod::GroupPooling:
            os << "Group Pooling";
            break;
        default:
            os.setstate(std::ios_base::failbit);
    }
    return os;
}

/**
 \brief Implementation of optional stopping.
 
 This will use two parameters set at construction of the OptionalStopping class, `_n_trials` and `_n_new_obs`
 for every trial, the routine will add `_n_new_obs` to all groups, recalculate the statistics, and run the test. It will then select an outcome based on researcher's preference and check it's significance. If
 the results is significant, it'll not make a new attempt to add more data, and will return to the hack() routine.
 */
void OptionalStopping::perform(Experiment* experiment, DecisionStrategy* decisionStrategy) {
    
    if (VERBOSE) std::cout << "Optional Stopping...\n";
    
    for (int t = 0; t < _n_attempts && t < _max_attempts ; t++) {
        
        addObservations(experiment, _num);
        
        // TODO: This can still be done nicer
        experiment->calculateStatistics();
        experiment->calculateEffects();
        experiment->runTest();
        
        if (decisionStrategy->verdict(*experiment, DecisionStage::WhileHacking))
            return;
    }
    
    

}

void OptionalStopping::addObservations(Experiment *experiment, const int &n) {
    auto new_observations = experiment->dataStrategy->genNewObservationsForAllGroups(experiment, n);
    
    int i = 0;
    std::for_each(experiment->measurements.begin(), experiment->measurements.end(),
                  [&new_observations, &i](arma::Row<double> &row) {
                      row.insert_cols(row.size(), new_observations[i++]);
                  });
    
    
}




/**
 \brief Implementation of Outliers Removal based on an item's distance from the \mu.
 
 The `_sd_multiplier`, d is set at the construction, and it'll be used to check whether a measurement
 should be removed or not. Any item satisfying the  v > |\mu - d * \sigma|  will be removed from the
 dataset.
 */
void SDOutlierRemoval::perform(Experiment* experiment, DecisionStrategy* decisionStrategy){
    
    if (VERBOSE) std::cout << "Outliers Removal...\n";
    
    int res = 0;
    
    for (auto &d : _multipliers) {
        
        for (int t = 0; t < _n_attempts &&
                        t < _max_attempts &&
                        res != 1
                        ; t++) {
            
            res = removeOutliers(experiment, _num, d);
            
            experiment->calculateStatistics();
            experiment->calculateEffects();
            experiment->runTest();
            
            
            if (decisionStrategy->verdict(*experiment, DecisionStage::WhileHacking))
                return ;
            
        }
    }
    std::cout << std::endl;
    
}


int SDOutlierRemoval::removeOutliers(Experiment *experiment, const int &n, const int &d) {
    int g = 0;     // Only to access the means, vars
    
    int res = 0;
    
    for (auto &row : experiment->measurements) {
        
        // At least one row has less than `_min_observations`
        if (row.size() <= _min_observations)
            res = 1;
            
        
        // This trick makes finding the largest outlier easier. I'll see if I can find a better way
        if (_order == "max first")
            row = sort(row);

        // Finding the outliers
        arma::uvec inx = arma::find(row < (experiment->means[g] - d * sqrt(experiment->vars[g]))
                                    ||
                                    row > (experiment->means[g] + d * sqrt(experiment->vars[g])));
        

        for (int i = inx.size() - 1;
             i >= 0 && (abs((int)inx.size() - n) <= i) && row.size() > _min_observations ;
             i--)
        {
            row.shed_col(inx[i]);
            
            // Shifting the index back
            inx = inx - 1;
        }
        
        g++;
    }
    
    // Success Code
    return res;
}



/**
 A Factory method for building hacking strategies
 
 \sa README.md

 @param config A JSON object defining a hacking strategy, and its parameters
 @return Pointer to a HackingStrategy
 */
HackingStrategy *HackingStrategy::buildHackingMethod(json& config) {

    if (config["name"] == "Optional Stopping"){
        return new OptionalStopping(config["level"],
                                    config["num"],
                                    config["n_attempts"],
                                    config["max_attempts"]);

    }else if (config["name"] == "SD Outlier Removal") {
        return new SDOutlierRemoval(config["level"],
                                    config["order"],
                                    config["num"],
                                    config["n_attempts"],
                                    config["max_attempts"],
                                    config["min_observations"],
                                    config["multipliers"]);

    }else if (config["name"] == "Group Pooling") {
        return new GroupPooling(config["num"]);
    }else{
        throw std::invalid_argument("Cannot recognize the p-hacking method.");
    }
    
}


/**
 Perform Group Pooling on the given Experiment.

 @param experiment A pointer to researcher's experiment
 @param decisionStrategy A pointer to researcher's decision strategy
 */
void GroupPooling::perform(Experiment *experiment, DecisionStrategy *decisionStrategy) {
    
    if (VERBOSE) std::cout << "Group Pooling...\n";
    
    if (experiment->setup.nc >= 2){
        
        // Length of each permutation
        const int r = _num;
        
        // Original number of conditions
        const int n = experiment->setup.nc;
        
        // Filling a range(0, n)
        std::vector<int> v(n);
        std::iota(v.begin(), v.end(), 0);
        
        // Gets the list of all permutation
        std::vector<std::vector<int>>
        permutations = for_each_reversible_circular_permutation(v.begin(),
                                                                 v.begin() + r,
                                                                 v.end(),
                                                                 collector());
        
        std::vector<arma::Row<double>> pooled_groups;
        for (auto &per : permutations) {
                        
            for (int d = 0; d < experiment->setup.nd; d++) {
                
                // Creating an empty new group
                experiment->measurements.push_back(arma::Row<double>());
                
                for (int i = 0; i < r ; i++){
                    
                    // Fill the new group by pooling members of the selected permutation, `per`.
                    experiment->measurements.back().insert_cols(experiment->measurements.back().size(),
                                                                experiment->measurements[per[i] * (n - 1) + d]);
                    
                    
                }
                
            }
            
        }
        
        
        // TODO: Improve me! This is very ugly and prune to error
        int new_ng = experiment->measurements.size();
        experiment->initResources(new_ng);
        
        experiment->calculateStatistics();
        experiment->calculateEffects();
        experiment->runTest();
        
        if (decisionStrategy->verdict(*experiment, DecisionStage::WhileHacking)){
            return ;
        }
        
        
    }else{
        throw std::domain_error("There is not enough groups for pooling.");
    }

}
