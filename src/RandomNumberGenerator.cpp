//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include "RandomNumberGenerator.h"

#include <numeric>
#include <algorithm>
#include <RandomNumberGenerator.h>
#include "Utilities.h"

#include "gsl/gsl_rng.h"


arma::Row<double>
RandomNumberGenerator::normal(const double &mean, const double &sd, const double &n){
    return sd * arma::randn<arma::Row<double>>(n) + mean;
}

// arma
std::vector<arma::Row<double> >
RandomNumberGenerator::normal(const arma::Row<double>& means, const arma::Row<double>& sds, const int &n) {

    std::vector<arma::Row<double>> rns;

    for (int i = 0; i < means.size(); ++i){
        rns.push_back(normal(means[i], sds[i], n));
    }

    return rns;
}

// arma 
std::vector<arma::Row<double>>
RandomNumberGenerator::normal(const arma::Row<double> &means, const arma::Row<double> &sds, const arma::Row<int> &nobs){
    std::vector<arma::Row<double>> rns;
    
    // TODO: Replace the for loop with a stl algorithm
    for (int i = 0; i < means.size(); ++i){
//        rns[i] = arma::randn(nobs[i]);
        // rns.push_back(sds[i] * arma::randn<arma::Row<double>>(nobs[i]) + means[i]);
        rns.push_back(normal(means[i], sds[i], nobs[i]));
    }
    
    return rns;
}

// arma
std::vector<arma::Row<double> >
RandomNumberGenerator::mvnorm(const arma::Row<double> &means, const arma::Mat<double> &sigma, const int &n){
    arma::mat rans_mat = arma::mvnrnd(means.t(), sigma, n);
    
    std::vector<arma::Row<double> > rans;
    for (int i = 0 ; i < means.size(); i ++) {
        rans.push_back(rans_mat.row(i));
    }
    
    return rans;
}

// arma
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
    return gsl_ran_flat(rng_stream, 0, 1);
}

double RandomNumberGenerator::uniform(const double &min, const double &max){
    return gsl_ran_flat(rng_stream, min, max);
}


/**
 Return a value between \f$[a, b)\f$ or \f$[b, c)\f$ based on the outcome of a Bernoulli trial with
 probability of \f$p\f$. The return value has \f$p\f$ chance of being in \f$[a, b)\f$ and \f$1-p\f$
 chance of being in \f$[b, c)\f$.
 */

//\param p Bernoulli trial probability
//\param a Lower bound
//\param b Middle bound
//\param c Upper bound
//\return A value between [a, c)

int RandomNumberGenerator::genSampleSize(const double &p, const double &a, const double &b, const double &c){
    if (gsl_ran_bernoulli(rng_stream, p)) {
        return static_cast<int>(gsl_ran_flat(rng_stream, a, b));
    }else{
        return static_cast<int>(gsl_ran_flat(rng_stream, b, c));
    }
}
