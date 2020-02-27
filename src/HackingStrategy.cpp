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

#include "spdlog/spdlog.h"
#include "utils/permutation.h"
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
        
    }else if (hacking_strategy_config["_name"] == "SubjectiveOutlierRemoval") {
    
        auto params = hacking_strategy_config.get<SubjectiveOutlierRemoval::Parameters>();
        return std::make_unique<SubjectiveOutlierRemoval>(params);
    
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
    std::for_each(experiment->measurements.begin(), experiment->measurements.end(),
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
 
 */
void SDOutlierRemoval::perform(Experiment* experiment, DecisionStrategy* decisionStrategy){
    
    if (FLAGS::VERBOSE)
        std::cout << ">>> Outliers Removal...\n";
    
    spdlog::info("Outliers Removal");
    
    /// result flag
    int res = 0;
    
    for (auto d : params.multipliers) {
        
        for (int t = 0; t < params.n_attempts &&
                        t < params.max_attempts &&
                        res != 1
                        ; t++) {
            
            res = this->removeOutliers(experiment, params.num, d);
            
            experiment->calculateStatistics();
            experiment->calculateEffects();
            experiment->runTest();
            
            
            if (!decisionStrategy->verdict(*experiment, DecisionStage::WhileHacking).isStillHacking())
                return ;
            
        }
    }
    
    if (FLAGS::VERBOSE)
        std::cout << ">>> Done Removing Outliers...\n";
    
}


int SDOutlierRemoval::removeOutliers(Experiment *experiment,
                                     const int n, const double d) {

    int res = 0;
        
    arma::rowvec standaraized;
    
    /// Removing outliers only from the original groups, see #104
    /// This is addressing the GroupPooling, and it doesn't have to do anything with outlier removal
    /// outlier removal is still being done in all groups
    for (int i = 0; i < experiment->setup.ng(); ++i) {
        
        auto &row = experiment->measurements[i];
        
        // At least one row has less than `min_observations`
        if (row.size() <= params.min_observations)
            return 1;     // Unsuccessful retrun, nothing has removed.
                          // TODO: I can improve this with `std::optional`
            
        
        // This trick makes finding the largest outlier easier. I'll see if I can find a better way
        if (params.order == "max first")
            row = sort(row);

        
        standaraized = arma::abs(row - experiment->means[i]) / experiment->stddev[i];
                
        // Finding the outliers, returning only `n` of them
        arma::uvec inx = arma::find(standaraized > d, n, "first");
                
        if ((row.n_elem - inx.n_elem) <= params.min_observations)
            inx = inx.head(row.n_elem - params.min_observations);
        
        row.shed_cols(inx);
        
    }
    
    // Success Code
    return res;
}


/**
 Perform subjective outliers removal on an Experiment. There is not much difference between
 this routine and the SDOutlierRemoval, mainly, I generate the Ks beforehand, and the entire
 procedure is technically one hacking step.
 
 */
void SubjectiveOutlierRemoval::perform(Experiment *experiment, DecisionStrategy *decision_strategy){
    
    if (FLAGS::VERBOSE)
        std::cout << ">>> Subjective Outliers Removal...\n";
    
    /// Creating a regularly spaced vector of values with the given start, step, end.
    static arma::vec Ks = arma::regspace<arma::vec>(params.range[1], -1. * params.step_size, params.range[0]);
    
    /// Going through K's in order and return as soon as the deicsion_strategy is satisfied.
    for (auto &k : Ks) {
        
        /// Removing the outliers from control groups as well.
        for (int i{0}; i < experiment->setup.ng(); ++i){
            auto &row = experiment->measurements[i];
            
            arma::rowvec standaraized = arma::abs(row - experiment->means[i]) / experiment->stddev[i];
                    
            /// Finding the outliers
            arma::uvec inx = arma::find(standaraized > k);
            
            /// Keeping at least `min_observations`
            if ((row.n_elem - inx.n_elem) <= params.min_observations)
                inx = inx.head(row.n_elem - params.min_observations);
            
            row.shed_cols(inx);
        }
        
        experiment->calculateStatistics();
        experiment->calculateEffects();
        experiment->runTest();
        
        if (!decision_strategy->verdict(*experiment, DecisionStage::WhileHacking).isStillHacking()){
            return ;
        }
        
    }
    
    
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
