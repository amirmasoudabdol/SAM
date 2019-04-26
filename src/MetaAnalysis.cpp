//
// Created by Amir Masoud Abdol on 2019-04-25.
//

#include <iostream>
#include <algorithm>

#include "MetaAnalysis.h"

#include <mlpack/core.hpp>
#include <mlpack/methods/neighbor_search/neighbor_search.hpp>

#include <mlpack/core.hpp>
#include <mlpack/methods/linear_regression/linear_regression.hpp>

using namespace mlpack;
using namespace mlpack::regression;

using namespace sam;
using namespace std;


arma::vec FixedEffectEstimator::estimate(vector<Submission> publications) {
    
    arma::rowvec predictors(publications.size());
    
    arma::rowvec responses(publications.size());
    arma::rowvec weights(publications.size());
    arma::rowvec predictions(publications.size());
    
    for (int i = 0; i < publications.size(); i++) {
        predictors(i) = 0;
        responses(i) = publications[i].yi;
        weights(i) = 1. / publications[i].vi;
    }
    
    LinearRegression fixed_model(predictors, responses, weights);
    
    return fixed_model.Parameters();
    
}


arma::vec RandomEffectEstimator::estimate(vector<Submission> publications) {
    
    return arma::vec();
}
