//
// Created by Amir Masoud Abdol on 2019-04-25.
//

#include <iostream>
#include <algorithm>

#include "MetaAnalysis.h"

using namespace mlpack;
using namespace mlpack::regression;

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
        responses(i) = publications[i].mean;
        weights(i) = 1. / publications[i].var;
    }

    LinearRegression fixed_model(predictors, responses, weights);

    return fixed_model.Parameters();
    
}


arma::vec RandomEffectEstimator::estimate(vector<Submission> publications) {
    
    arma::mat predictors(2, publications.size());
    
    arma::rowvec responses(publications.size());
    arma::rowvec weights(publications.size());
    arma::rowvec predictions(publications.size());
    
    for (int i = 0; i < publications.size(); i++) {
        predictors(0, i) = 1;
        predictors(1, i) = publications[0].mean;
        responses(i) = publications[i].mean;
        weights(i) = 1. / publications[i].var;
    }
    
    LinearRegression random_model(predictors, responses, weights);
    
    return random_model.Parameters();
    
}
