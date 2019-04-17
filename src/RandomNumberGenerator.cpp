//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include "RandomNumberGenerator.h"

#include <numeric>
#include <algorithm>
#include <RandomNumberGenerator.h>
#include "Utilities.h"

arma::Row<double>
RandomNumberGenerator::normal(const double &mean, const double &var, const double &n){
    return var * arma::randn<arma::Row<double>>(n) + mean;
}

std::vector<arma::Row<double> >
RandomNumberGenerator::normal(const arma::Row<double>& means, const arma::Row<double>& vars, const int &n) {

    std::vector<arma::Row<double>> rns;

    for (int i = 0; i < means.size(); ++i){
        rns.push_back(normal(means[i], vars[i], n));
    }

    return rns;
}

std::vector<arma::Row<double>>
RandomNumberGenerator::normal(const arma::Row<double> &means, const arma::Row<double> &vars, const arma::Row<int> &nobs){
    std::vector<arma::Row<double>> rns;
    
    // TODO: Replace the for loop with a stl algorithm
    for (int i = 0; i < means.size(); ++i){
//        rns[i] = arma::randn(nobs[i]);
        // rns.push_back(sds[i] * arma::randn<arma::Row<double>>(nobs[i]) + means[i]);
        rns.push_back(normal(means[i], vars[i], nobs[i]));
    }
    
    return rns;
}

std::vector<arma::Row<double> >
RandomNumberGenerator::mvnorm(const arma::Row<double> &means, const arma::Mat<double> &sigma, const int &n){
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
    arma::mat rans_mat = arma::mvnrnd(means.t(), sigma, arma::max(nobs));
    
    std::vector<arma::Row<double> > rans;
    for (int i = 0 ; i < means.size(); i ++) {
        rans.push_back(rans_mat.row(i));
    }
    
    return rans;
}

double RandomNumberGenerator::uniform() {
    return uniform(0, 1);
}

double RandomNumberGenerator::uniform(const double &min, const double &max){
    uniformDist.param(std::uniform_real_distribution<double>::param_type(min, max));
    return uniformDist(gen);
}


/**
 Return a value between [a, b) or [b, c) based on the outcome of a Bernoulli trial with
 probability of p. The return value has p chance of being in [a, b) and 1-p
 chance of being in [b, c).
 */

//@param p Bernoulli trial probability
//@param a Lower bound
//@param b Middle bound
//@param c Upper bound
//\return A value between [a, c)

int RandomNumberGenerator::genSampleSize(const double &p, const double &a, const double &b, const double &c){
    bernoulliDist.param(std::bernoulli_distribution::param_type(p));
    if (bernoulliDist(gen)) {
        return uniform(a, b);
    }else{
        return uniform(b, c);
    }
}

int RandomNumberGenerator::genSampleSize(const std::vector<double> &intervals, const std::vector<double> &weights){
    piecewiseConstDist.param((std::piecewise_constant_distribution<>::param_type(intervals.begin(), intervals.end(), weights.begin())));
    return piecewiseConstDist(gen);
}
