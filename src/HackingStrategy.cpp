//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <HackingStrategy.h>
#include <Experiment.h>
#include <DecisionStrategy.h>

#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <stdexcept>
#include <random>

#include "utils/permutation.h"
#include "utils/magic_enum.hpp"

#include "sam.h"

using namespace sam;

HackingStrategy::~HackingStrategy() {
    // Pure deconstructor
};

/**
 A Factory method for building hacking strategies
 
 @param config A JSON object defining a hacking strategy, and its parameters
 @return Pointer to a HackingStrategy
 */
std::unique_ptr<HackingStrategy> HackingStrategy::build(json &hacking_strategy_config) {
    
    if (hacking_strategy_config["name"] == "OptionalStopping"){
        return std::make_unique<OptionalStopping>(hacking_strategy_config["level"],
                                    hacking_strategy_config["num"],
                                    hacking_strategy_config["n_attempts"],
                                    hacking_strategy_config["max_attempts"]);
        
    }else if (hacking_strategy_config["name"] == "SDOutlierRemoval") {
        return std::make_unique<SDOutlierRemoval>(hacking_strategy_config["level"],
                                    hacking_strategy_config["order"],
                                    hacking_strategy_config["num"],
                                    hacking_strategy_config["n_attempts"],
                                    hacking_strategy_config["max_attempts"],
                                    hacking_strategy_config["min_observations"],
                                    hacking_strategy_config["multipliers"]);
        
    }else if (hacking_strategy_config["name"] == "GroupPooling") {
        return std::make_unique<GroupPooling>(hacking_strategy_config["nums"]);
    }else if (hacking_strategy_config["name"] == "ConditionDropping") {
        return std::make_unique<ConditionDropping>();
    }else if (hacking_strategy_config["name"] == "NoHack") {
        return std::make_unique<NoHack>();
    }else{
        throw std::invalid_argument("Cannot recognize the p-hacking method.");
    }
    
}

std::unique_ptr<HackingStrategy> HackingStrategy::build(HackingMethod method) {
    switch (method) {
        
        case HackingMethod::NoHack:
            return std::make_unique<NoHack>();
            break;
        case HackingMethod::OptionalStopping:
            return std::make_unique<OptionalStopping>();
            break;
        case HackingMethod::SDOutlierRemoval:
            return std::make_unique<SDOutlierRemoval>();
            break;
        case HackingMethod::GroupPooling:
            return std::make_unique<GroupPooling>();
            break;
        case HackingMethod::ConditionDropping:
            return std::make_unique<ConditionDropping>();
            break;
        default:
            return std::make_unique<NoHack>();
            break;
    }
}

std::unique_ptr<HackingStrategy> HackingStrategy::build(HackingStrategyParameters &hsp) {

    switch (hsp.name) {
        case HackingMethod::NoHack:
            return std::make_unique<NoHack>();
            break;
        case HackingMethod::OptionalStopping:
            return std::make_unique<OptionalStopping>();
            break;
        case HackingMethod::SDOutlierRemoval:
            return std::make_unique<SDOutlierRemoval>();
            break;
        case HackingMethod::GroupPooling:
            return std::make_unique<GroupPooling>();
            break;
        case HackingMethod::ConditionDropping:
            return std::make_unique<ConditionDropping>();
            break;
        default:
            throw std::invalid_argument("Cannot recognize the p-hacking method.");
            break;
    }
}

/**
 @brief Implementation of optional stopping.
 
 This will use two parameters set at construction of the OptionalStopping class,
 `n_trials` and `n_new_obs` for every trial, the routine will add `n_new_obs`
 to all groups, recalculate the statistics, and run the test. It will then
 select an outcome based on researcher's preference and check it's significance.
 If the results is significant, it'll not make a new attempt to add more data,
 and will return to the hack() routine.
 */
void OptionalStopping::perform(Experiment* experiment, DecisionStrategy* decisionStrategy) {
    
    if (FLAGS::VERBOSE) std::cout << "Optional Stopping...\n";
    
    for (int t = 0; t < n_attempts && t < max_attempts ; t++) {
        
        addObservations(experiment, num);
        
        // TODO: This can still be done nicer
        experiment->calculateStatistics();
        experiment->calculateEffects();
        experiment->runTest();
        
        if (decisionStrategy->verdict(*experiment, DecisionStage::WhileHacking))
            return;
    }
    
    

}

void OptionalStopping::addObservations(Experiment *experiment, const int &n) {
    auto new_observations = experiment->data_strategy->genNewObservationsForAllGroups(experiment, n);
    
    int i = 0;
    std::for_each(experiment->measurements.begin(), experiment->measurements.end(),
                  [&new_observations, &i](arma::Row<double> &row) {
                      row.insert_cols(row.size(), new_observations[i++]);
                  });    
    
}

void OptionalStopping::randomize(int min_n = 1, int max_n = 10) {
    level = "dv";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> uniform(min_n, max_n);

    num = uniform(gen);

    uniform.param(std::uniform_int_distribution<>::param_type(1, 3));
    n_attempts = uniform(gen);

    max_attempts = 10;
}



/**
 @brief Implementation of Outliers Removal based on an item's distance from the \mu.
 
 The `sd_multiplier`, d is set at the construction, and it'll be used to check whether a measurement
 should be removed or not. Any item satisfying the  \f$v > |\mu - d * \sigma|\f$  will be removed from the
 dataset.
 */
void SDOutlierRemoval::perform(Experiment* experiment, DecisionStrategy* decisionStrategy){
    
    if (FLAGS::VERBOSE) std::cout << "Outliers Removal...\n";
    
    int res = 0;
    
    for (auto &d : multipliers) {
        
        for (int t = 0; t < n_attempts &&
                        t < max_attempts &&
                        res != 1
                        ; t++) {
            
            res = removeOutliers(experiment, num, d);
            
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
        
        // At least one row has less than `min_observations`
        if (row.size() <= min_observations)
            res = 1;
            
        
        // This trick makes finding the largest outlier easier. I'll see if I can find a better way
        if (order == "max first")
            row = sort(row);

        // Finding the outliers
        arma::uvec inx = arma::find(row < (experiment->means[g] - d * sqrt(experiment->vars[g]))
                                    ||
                                    row > (experiment->means[g] + d * sqrt(experiment->vars[g])));
        

        for (int i = inx.size() - 1;
             i >= 0 && (abs((int)inx.size() - n) <= i) && row.size() > min_observations ;
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
 Perform Group Pooling on the given Experiment.

 @param experiment A pointer to researcher's experiment
 @param decisionStrategy A pointer to researcher's decision strategy
 */
void GroupPooling::perform(Experiment *experiment, DecisionStrategy *decisionStrategy) {
    
    if (FLAGS::VERBOSE) std::cout << "Group Pooling...\n";
    
    if (experiment->setup.nc() < 2){
        throw std::domain_error("There is not enough groups for pooling.");
    }
    
    for (auto &r : nums){
        pool(experiment, r);
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

}

void GroupPooling::pool(Experiment *experiment, int r){
    // Length of each permutation
    //    const int r = _num;
    
    // Original number of conditions
    const int n = experiment->setup.nc();
    
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
        
        for (int d = 0; d < experiment->setup.nd(); d++) {
            
            // Creating an empty new group
            experiment->measurements.push_back(arma::Row<double>());
            
            for (int i = 0; i < r ; i++){
                
                // Fill the new group by pooling members of the selected permutation, `per`.
                experiment->measurements.back().insert_cols(experiment->measurements.back().size(),
                                                            experiment->measurements[per[i] * (n - 1) + d]);
                
                
            }
            
        }
        
    }
}
