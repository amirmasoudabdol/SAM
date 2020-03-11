//
// Created by Amir Masoud Abdol on 2019-04-25.
//

#include <iostream>
#include <algorithm>

#include "MetaAnalysis.h"

// #include <mlpack/core.hpp>
// #include <mlpack/methods/neighbor_search/neighbor_search.hpp>
// #include <mlpack/methods/linear_regression/linear_regression.hpp>

// using namespace mlpack;
// using namespace mlpack::regression;

using namespace std;
using namespace sam;


MetaAnalysis::~MetaAnalysis() { 

};


std::unique_ptr<MetaAnalysis> MetaAnalysis::build(std::string name) {
    if (name == "FixedEffectEstimator")
    {
        return std::make_unique<FixedEffectEstimator>();
    }

    return nullptr;
}


arma::vec FixedEffectEstimator::estimate(vector<Submission> publications) {
    
    arma::mat predictors(1, publications.size());

    arma::rowvec responses(publications.size());
    arma::rowvec weights(publications.size());
    arma::rowvec predictions(publications.size());

    for (int i = 0; i < publications.size(); i++) {
        predictors(0, i) = 0;
        responses(i) = publications[i].group_.mean_;
        weights(i) = 1. / publications[i].group_.var_;
    }

    // LinearRegression fixed_model(predictors, responses, weights);

    // return fixed_model.Parameters();
    return arma::vec();
    
}


arma::vec RandomEffectEstimator::estimate(vector<Submission> publications) {
    
    arma::mat predictors(2, publications.size());
    
    arma::rowvec responses(publications.size());
    arma::rowvec weights(publications.size());
    arma::rowvec predictions(publications.size());
    
    for (int i = 0; i < publications.size(); i++) {
        predictors(0, i) = 1;
        predictors(1, i) = publications[0].group_.mean_;
        responses(i) = publications[i].group_.mean_;
        weights(i) = 1. / publications[i].group_.var_;
    }
    
    // LinearRegression random_model(predictors, responses, weights);
    
    // return random_model.Parameters();
    return arma::vec();
    
}
