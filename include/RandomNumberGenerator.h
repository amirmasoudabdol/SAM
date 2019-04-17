//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_RANDOMNUMBERGENERATOR_H
#define SAMPP_RANDOMNUMBERGENERATOR_H

#include <vector>
#include <string>
#include <armadillo>
#include <random>
#include "gsl/gsl_rng.h"
#include "gsl/gsl_randist.h"


class RandomNumberGenerator {

public:
    
    std::random_device rd;
    std::mt19937 gen;
    std::bernoulli_distribution bernoulliDist;
    std::uniform_real_distribution<double> uniformDist;
    std::piecewise_constant_distribution<> piecewiseConstDist;
    

    RandomNumberGenerator(int seed) :
        _seed(seed)
    {
        gen = std::mt19937(rd());
        gen.seed(_seed);
        
        arma::arma_rng::set_seed(_seed);
            
    };
    
    ~RandomNumberGenerator() {

    };

    void setSeed(int seed) {
        _seed = seed;
    };
    int getSeed() {
        return _seed;
    };

    double uniform();
    double uniform(const double &min, const double &max);
    
    int genSampleSize(const double &, const double &, const double &, const double &);
    int genSampleSize(const std::vector<double> &intervals, const std::vector<double> &weights);
    
    // normal distribution
    arma::Row<double>
    normal(const double &mean, const double &var, const double &n);
    std::vector<arma::Row<double> >
    normal(const arma::Row<double> &means, const arma::Row<double> &vars, const int &n);
    std::vector<arma::Row<double> >
    normal(const arma::Row<double> &means, const arma::Row<double> &vars, const arma::Row<int> &nobs);

    // multivariate normal distribution
    std::vector<arma::Row<double> >
    mvnorm(const arma::Row<double> &means, const arma::Mat<double> &sigma, const int &n);
    std::vector<arma::Row<double> >
    mvnorm(const arma::Row<double> &means, const arma::Mat<double> &sigma, const arma::Row<int> &nobs);
    

private:
    int _seed;
};

#endif //SAMPP_RANDOMNUMBERGENERATOR_H
