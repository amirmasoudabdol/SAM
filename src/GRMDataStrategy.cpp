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
    
    /// I'm hardcoding this to only draw difficulties from one
    /// distribution. If there'll be an adaptation of the model,
    /// I probably need to make some changes here.
    if (params.diff_dists)
        betas.imbue([&]() { return Random::get(params.diff_dists.value()[0]); });
    
    for (int g{0}; g < experiment->setup.ng(); ++g) {
        experiment->measurements[g].resize(experiment->setup.nobs()[g]);
        
        double theta {0};
        do {
            experiment->measurements[g].imbue(
                [&](){
                    /// Improvement: It is probably a good idea to make a function out of this, and return arma::Row<>
                    if (params.abil_dists)
                        theta = Random::get(params.abil_dists.value()[g]);
                    
                    return generate_sum_of_scores(theta);
                });
        
        // This makes sure that I don't have a totally unanswered test
        } while (!arma::any(experiment->measurements[g]));
        
    }
    
}

// Generate persons j answer to all items
double GRMDataStrategy::generate_sum_of_scores(const double theta) {

    poa = (arma::exp(theta - betas)) / (1 + arma::exp(theta - betas));

    urand.imbue([&]() {return Random::get(uniform_dist); });
    
    responses = poa > urand;
            
    return arma::accu(responses);
}

std::vector<arma::Row<double> >
GRMDataStrategy::genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs){
    
    std::vector<arma::Row<double> > new_values(experiment->setup.ng());
    
    for (int g{0}; g < experiment->setup.ng(); ++g) {
        new_values[g].resize(n_new_obs);
        
        thetas.resize(params.n_items, params.n_categories);
        thetas.imbue([&]() { return Random::get(params.abil_dists.value()[g] ); });
        
        new_values[g].imbue([&](){
            auto theta = Random::get(params.abil_dists.value()[g]);
            return generate_sum_of_scores(theta);
        });
    }
    
    return new_values;
    
}

arma::Row<double>
GRMDataStrategy::genNewObservationsFor(Experiment* experiment, int g, int n_new_obs){
    return arma::Row<double>();
}
