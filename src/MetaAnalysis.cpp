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


arma::vec FixedEffectEstimator::estimate(vector<Submission> publications) {
    
    arma::rowvec predictors(publications.size());
    
    arma::rowvec responses(publications.size());
    arma::rowvec weights(publications.size());
    arma::rowvec predictions(publications.size());
    
    for (int i = 0; i < publications.size(); i++) {
        predictors(i) = 0;
        responses(i) = publications[i].mean;
        weights(i) = 1. / publications[i].var;
    }
    
    LinearRegression fixed_model(predictors, responses, weights);
    
    return fixed_model.Parameters();
    
}


arma::vec RandomEffectEstimator::estimate(vector<Submission> publications) {
    
    return arma::vec();
}
