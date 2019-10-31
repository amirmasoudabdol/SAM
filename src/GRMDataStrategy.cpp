//
// Created by Amir Masoud Abdol on 2019-10-29
//

#include <iostream>
#include <string>

#include "Experiment.h"
#include "ExperimentSetup.h"
#include "DataStrategy.h"

using namespace sam;


void GRMDataStrategy::genData(Experiment* experiment) {
    
    
    difficulty_dist = std::normal_distribution<>{};
    ability_dist = std::normal_distribution<>{};
    uniform_dist = std::uniform_real_distribution<>{};
    
    int n_obs {25};
    int n_items {10};            // number of items
    int n_categories {4};        // number of categories in likert
    
    betas.resize(n_items, n_categories);
    betas.imbue([&]() { return Random::get(difficulty_dist); });
    
//    std::cout << betas << std::endl;
    
    poa.resize(n_items, n_categories);
    responses.resize(n_items, n_categories);
    urand.resize(n_items, n_categories);
    scores.resize(n_items, 1);
    sumofscores.resize(n_obs, 1);
    
//    // calculating scores aggregate for each person
//    for (int j{0};  j < n_obs; ++j) {
//
//        double theta_j = Random::get(ability_dist);
//
//        poa = (arma::exp(theta_j - betas)) / (1 + arma::exp(theta_j - betas));
//
//        urand.imbue([&]() {return Random::get(uniform_dist); });
//
//        responses = poa > urand;
//
//        scores = arma::sum(responses, 1);
//
//        sumofscores.at(j, 0) = arma::accu(scores);
//
////        sumofscores
//    }
//
//
//    // This is basically going to fill all the groups
////    std::generate(experiment->measurements.begin(),
////                  experiment->measurements.end(),
////                  [this]() {return arma::Row<double>().im; });
//
    for (auto &dv : experiment->measurements) {
        dv.resize(experiment->setup.nobs().max());
        dv.imbue([&](){ return generate_sum_of_scores(); });
//        std::cout << dv << std::endl;
    }
    

}

// Generate persons j answer to all items
arma::umat GRMDataStrategy::generate_binary_scores() {
    double theta_j = Random::get(ability_dist);
        
    poa = (arma::exp(theta_j - betas)) / (1 + arma::exp(theta_j - betas));

    urand.imbue([&]() {return Random::get(uniform_dist); });
    
    return poa > urand;
}

double GRMDataStrategy::generate_sum_of_scores() {
    responses = generate_binary_scores();
    return arma::accu(responses);
}


void GRMDataStrategy::generate_betas() {
//    betas =
}


arma::mat GRMDataStrategy::generate_thetas() {
    
}

std::vector<arma::Row<double> >
GRMDataStrategy::genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs){}

arma::Row<double>
GRMDataStrategy::genNewObservationsFor(Experiment* experiment, int g, int n_new_obs){}
