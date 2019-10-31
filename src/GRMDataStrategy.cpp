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
    
    for (int g{0}; g < experiment->setup.ng(); ++g) {
        experiment->measurements[g].resize(experiment->setup.nobs()[g]);
        experiment->measurements[g].imbue(
            [&](){
                // TODO: Replace this with `abilities_dist` for better performance
                auto theta = Random::get<std::normal_distribution<>>(experiment->setup.abilities.values[g]);
                return generate_sum_of_scores(theta);
            });
    }
    
}

// Generate persons j answer to all items
arma::umat GRMDataStrategy::generate_binary_scores(const double theta) {
//    double theta_j = Random::get(abilities_dist);
        
    poa = (arma::exp(theta - betas)) / (1 + arma::exp(theta - betas));

    urand.imbue([&]() {return Random::get(uniform_dist); });
    
    return poa > urand;
}

double GRMDataStrategy::generate_sum_of_scores(const double theta) {
    responses = generate_binary_scores(theta);
    return arma::accu(responses);
}

std::vector<arma::Row<double> >
GRMDataStrategy::genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs){}

arma::Row<double>
GRMDataStrategy::genNewObservationsFor(Experiment* experiment, int g, int n_new_obs){}
