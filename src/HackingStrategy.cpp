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
#include "effolkronium/random.hpp"

#include "sam.h"

using namespace sam;

using Random = effolkronium::random_static;

HackingStrategy::~HackingStrategy() {
    // Pure deconstructor
};

/**
 A Factory method for building hacking strategies
 
 @param config A JSON object defining a hacking strategy, and its parameters
 @return Pointer to a HackingStrategy
 */
std::unique_ptr<HackingStrategy> HackingStrategy::build(json &hacking_strategy_config) {
    
    if (hacking_strategy_config["_name"] == "OptionalStopping"){
        
        auto params = hacking_strategy_config.get<OptionalStopping::Parameters>();
        return std::make_unique<OptionalStopping>(params);
        
    }else if (hacking_strategy_config["_name"] == "SDOutlierRemoval") {
        
        auto params = hacking_strategy_config.get<SDOutlierRemoval::Parameters>();
        return std::make_unique<SDOutlierRemoval>(params);
        
    }else if (hacking_strategy_config["_name"] == "GroupPooling") {
        
        auto params = hacking_strategy_config.get<GroupPooling::Parameters>();
        return std::make_unique<GroupPooling>(params);
        
    }else if (hacking_strategy_config["_name"] == "ConditionDropping") {
        
        auto params = hacking_strategy_config.get<ConditionDropping::Parameters>();
        return std::make_unique<ConditionDropping>(params);
        
    }else if (hacking_strategy_config["_name"] == "NoHack") {
        
        return std::make_unique<NoHack>();
        
    }else{
        throw std::invalid_argument("Unknown Hacking Strategies.");
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
    
    for (int t = 0; t < params.n_attempts && t < params.max_attempts ; t++) {
        
        addObservations(experiment, params.num);
        
        // TODO: This can still be done nicer
        experiment->calculateStatistics();
        experiment->calculateEffects();
        experiment->runTest();
        
        if (!decisionStrategy->verdict(*experiment, DecisionStage::WhileHacking).isStillHacking())
            return;
    }
    
    

}

void OptionalStopping::addObservations(Experiment *experiment, const int &n) {
    
    // Get the new observations
    auto new_observations = experiment->data_strategy->genNewObservationsForAllGroups(experiment, n);
    
    int i = 0;
    
    // Add new observations to first `ng` group. I don't iterate over everything in
    // the `measurements` because they might actually be the results of `pooling`.
    // Therefore, I only add new values to original groups.
    std::for_each_n(experiment->measurements.begin(), experiment->setup.ng(),
                      [&new_observations, &i](arma::Row<double> &row) {
                          row.insert_cols(row.size(), new_observations[i++]);
                      });
    
}

void OptionalStopping::randomize(int min_n = 1, int max_n = 10) {
    params.level = "dv";

    params.num = Random::get<int>(min_n, max_n);

    params.n_attempts = Random::get<int>(1, 3);

    params.max_attempts = 10;
}



/**
 @brief Implementation of Outliers Removal based on an item's distance from the \mu.
 
 The `sd_multiplier`, d is set at the construction, and it'll be used to check
 whether a measurement should be removed or not. Any item satisfying the
 \f$v > |\mu - d * \sigma|\f$  will be removed from the dataset.
 */
void SDOutlierRemoval::perform(Experiment* experiment, DecisionStrategy* decisionStrategy){
    
    if (FLAGS::VERBOSE) std::cout << "Outliers Removal...\n";
    
    int res = 0;
    
    for (auto &d : params.multipliers) {
        
        for (int t = 0; t < params.n_attempts &&
                        t < params.max_attempts &&
                        res != 1
                        ; t++) {
            
            res = removeOutliers(experiment, params.num, d);
            
            experiment->calculateStatistics();
            experiment->calculateEffects();
            experiment->runTest();
            
            
            if (!decisionStrategy->verdict(*experiment, DecisionStage::WhileHacking).isStillHacking())
                return ;
            
        }
    }
    std::cout << std::endl;
    
}


int SDOutlierRemoval::removeOutliers(Experiment *experiment, const int &n, const int &d) {
    int g = 0;     // Only to access the means, vars
    
    int res = 0;
    
    arma::rowvec standaraized;
    
    // Removing outliers only from the original groups, see #104
    for (int i = 0; i < experiment->setup.ng(); ++i) {
        
        auto &row = experiment->measurements[i];
        
        // At least one row has less than `min_observations`
        if (row.size() <= params.min_observations)
            res = 1;
            
        
        // This trick makes finding the largest outlier easier. I'll see if I can find a better way
        if (params.order == "max first")
            row = sort(row);

        
        standaraized = arma::abs(row - experiment->means[i]) / experiment->stddev[i];
        
        // Finding the outliers
        arma::uvec inx = arma::find(standaraized > d);
                
        // TODO: I think this can be rewritten nicer
        for (int i = inx.size() - 1;
             i >= 0 && (abs((int)inx.size() - n) <= i) && row.size() > params.min_observations ;
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
        /// TODO: This should probably not be a throw and just a
        /// message. It's not a big deal after all, and I don't want to stop the program from running
        throw std::domain_error("There is not enough groups for pooling.");
    }
    
    // Pooling groups together
    for (auto &r : params.nums){
        pool(experiment, r);
    }
    
    // TODO: Improve me! This is very ugly and prune to error
    int new_ng = experiment->measurements.size();
    experiment->initResources(new_ng);
    
    experiment->calculateStatistics();
    experiment->calculateEffects();
    experiment->runTest();
    
    if (!decisionStrategy->verdict(*experiment, DecisionStage::WhileHacking).isStillHacking()){
        return ;
    }

}


/// Create a new group by pooling (adding) data from `r` groups together.
/// This literally appends the data of selected groups to each other to create a new group.
///
/// @param experiment a pointer to the given experiment. Note: This can be a copy of the experiment, based on the preferences of the researcher.
/// @param r Lenght of each permutation
void GroupPooling::pool(Experiment *experiment, int r){
    // Length of each permutation
    //    const int r = _num;
    
    // Original number of conditions
    const int n = experiment->setup.nc();
    
    // List of groups participating in the pool.
    std::vector<int> v(n);
    
    /// Note: This starts from `1` because the first group is the control group and
    /// it should not be included in the pooling. See #194. This is a bit different than before
    /// where I was using population parameters to run the t-test.
    std::iota(v.begin(), v.end(), 1); // Filling v with range(1, n)
    
    
    // Gets the list of all permutation
    std::vector<std::vector<int>>
    permutations = for_each_reversible_circular_permutation(v.begin(),
                                                            v.begin() + r,
                                                            v.end(),
                                                            collector());
    
    std::vector<arma::Row<double>> pooled_groups;
    
    // Extending the measurements to avoid over-pooling, see #104
    experiment->measurements.resize(experiment->setup.ng() + permutations.size() * experiment->setup.nd());
    
    for (auto &per : permutations) {
        
        for (int d = 0; d < experiment->setup.nd(); d++) {
            
            // Creating an empty new group
//            experiment->measurements.push_back(arma::Row<double>());
            
//            arma::Row<double> new_group{experiment->measurements[per[0]]};
            
            for (int i = 0; i < r ; i++){
                
//                new_group += experiment->measurements[per[i] * (n - 1) + d];
                experiment->measurements[experiment->setup.ng() + d]
                                .insert_cols(experiment->measurements.back().size(),
                                              experiment->measurements[per[i] * (n - 1) + d]);
                
            }
//            std::cout << new_group  << std::endl;
            // Fill the new group by pooling members of the selected permutation, `per`.
//            experiment->measurements[experiment->setup.ng() + d] = new_group;
            
        }
        
    }
}
