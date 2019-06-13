//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_RANDOMNUMBERGENERATOR_H
#define SAMPP_RANDOMNUMBERGENERATOR_H

#include <vector>
#include <string>
#include <random>

#include "sam.h"

namespace sam {

class RandomNumberGenerator {

    public:
        
//        std::random_device rd;
//        std::mt19937 gen;
//        std::bernoulli_distribution bernoulliDist;
//        std::piecewise_constant_distribution<> piecewiseConstDist;
    
    
        RandomNumberGenerator() {
//            seed = rand();
//            gen = std::mt19937(rd());
//            gen.seed(seed);
            
            arma::arma_rng::set_seed(seed);
        }

        RandomNumberGenerator(int s) :
            seed(s)
        {
//            gen = std::mt19937(rd());
//            gen.seed(seed);
            
            arma::arma_rng::set_seed(seed);
                
        };
        
        ~RandomNumberGenerator() {

        };

        void setSeed(int s) {
            seed = s;
        };
        int getSeed() {
            return seed;
        };
    
        // normal distribution
//        arma::Row<double>
//        normal(const double mean, const double var, const double n);
//        std::vector<arma::Row<double> >
//        normal(const arma::Row<double> &means, const arma::Row<double> &vars, const int n);
//        std::vector<arma::Row<double> >
//        normal(const arma::Row<double> &means, const arma::Row<double> &vars, const arma::Row<int> &nobs);

        // multivariate normal distribution
        std::vector<arma::Row<double> >
        mvnorm(const arma::Row<double> &means, const arma::Mat<double> &sigma, const int n);
        std::vector<arma::Row<double> >
        mvnorm(const arma::Row<double> &means, const arma::Mat<double> &sigma, const arma::Row<int> &nobs);
        

    private:
    int seed = 42;
    };
    
}

#endif //SAMPP_RANDOMNUMBERGENERATOR_H
