//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <numeric>
#include <algorithm>

#include "RandomNumberGenerator.h"

using namespace sam;

arma::Row<double>
RandomNumberGenerator::normal(const double mean, const double var, const double n){
    return var * arma::randn<arma::Row<double>>(n) + mean;
}

std::vector<arma::Row<double> >
RandomNumberGenerator::normal(const arma::Row<double> &means, const arma::Row<double> &vars, const int n) {

    std::vector<arma::Row<double> > rns;

    for (int i = 0; i < means.size(); ++i){
        rns.push_back(normal(means[i], vars[i], n));
    }

    return rns;
}

std::vector<arma::Row<double> >
RandomNumberGenerator::normal(const arma::Row<double> &means, const arma::Row<double> &vars, const arma::Row<int> &nobs){

    std::vector<arma::Row<double> > rns;
    
    // TODO: Replace the for loop with a stl algorithm
    for (int i = 0; i < means.size(); ++i){
        rns.push_back(normal(means[i], vars[i], nobs[i]));
    }

//    std::generate(rns.begin(), rns.end(), [&, i = 10]() mutable { i++; return normal(means[i], vars[i], nobs[i]); } );
    
    return rns;
}

std::vector<arma::Row<double> >
RandomNumberGenerator::mvnorm(const arma::Row<double> &means, const arma::Mat<double> &sigma, const int n){
    
    arma::mat rans_mat = arma::mvnrnd(means.t(), sigma, n);
    
    std::vector<arma::Row<double> > rans;
    for (int i = 0 ; i < means.size(); i ++) {
        rans.push_back(rans_mat.row(i));
    }
    
    return rans;
}

std::vector<arma::Row<double> >
RandomNumberGenerator::mvnorm(const arma::Row<double> &means, const arma::Mat<double> &sigma, const arma::Row<int> &nobs){
    
    // TODO: This can be optimized by some STL algorithm as well
    arma::mat rans_mat = arma::mvnrnd(means.t(), sigma, nobs.max());
    
    std::vector<arma::Row<double> > rans;
    for (int i = 0 ; i < means.size(); i ++) {
        rans.push_back(rans_mat.row(i));
    }
    
    return rans;
}

double RandomNumberGenerator::uniform() {
    return uniform(0, 1);
}

double RandomNumberGenerator::uniform(const double min, const double max){
    uniformDist.param(std::uniform_real_distribution<double>::param_type(min, max));
    return uniformDist(gen);
}

