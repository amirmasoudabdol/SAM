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
    
    sumofscores.resize(experiment->setup.nobs().max(), 1);
    
    betas.resize(params.n_items, params.n_categories);
    betas.imbue([&]() { return Random::get(difficulties_dist); });
    
    for (int g{0}; g < experiment->setup.ng(); ++g) {
        experiment->measurements[g].resize(experiment->setup.nobs()[g]);
        
        thetas.resize(params.n_items, params.n_categories);
        thetas.imbue([&]() { return Random::get<std::normal_distribution<>>(params.abilities[g] ); });
        
        experiment->measurements[g].imbue(
            [&](){
                // TODO: Replace this with `abilities_dist` for better performance
                // TODO: It is probably a good idea to make a function out of this, and return arma::Row<>
//                auto theta = Random::get<std::normal_distribution<>>(params.abilities[g]);
                return generate_sum_of_scores();
            });
    }
    
}

// Generate persons j answer to all items
arma::umat GRMDataStrategy::generate_binary_scores() {
//    double theta_j = Random::get(abilities_dist);
        
    poa = (arma::exp(thetas - betas)) / (1 + arma::exp(thetas - betas));

    urand.imbue([&]() {return Random::get(uniform_dist); });
    
    return poa > urand;
}

double GRMDataStrategy::generate_sum_of_scores() {
    responses = generate_binary_scores();
    return arma::accu(responses);
}

std::vector<arma::Row<double> >
GRMDataStrategy::genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs){
    
    std::vector<arma::Row<double> > new_values(experiment->setup.ng());
    
    for (int g{0}; g < experiment->setup.ng(); ++g) {
        new_values[g].resize(n_new_obs);
        
        thetas.resize(params.n_items, params.n_categories);
        thetas.imbue([&]() { return Random::get<std::normal_distribution<>>(params.abilities[g] ); });
        
        new_values[g].imbue([&](){
//            auto theta = Random::get<std::normal_distribution<>>(params.abilities[g]);
            return generate_sum_of_scores();
        });
    }
    
    return new_values;
    
}

arma::Row<double>
GRMDataStrategy::genNewObservationsFor(Experiment* experiment, int g, int n_new_obs){
    return arma::Row<double>();
}
