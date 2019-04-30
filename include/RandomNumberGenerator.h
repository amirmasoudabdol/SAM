//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_RANDOMNUMBERGENERATOR_H
#define SAMPP_RANDOMNUMBERGENERATOR_H

#include <vector>
#include <string>
#include <armadillo>
#include <random>

namespace sam {

class RandomNumberGenerator {

    public:
        
        std::random_device rd;
        std::mt19937 gen;
        std::bernoulli_distribution bernoulliDist;
        std::uniform_real_distribution<double> uniformDist;
        std::piecewise_constant_distribution<> piecewiseConstDist;
        

        RandomNumberGenerator(int s) :
            seed(s)
        {
            gen = std::mt19937(rd());
            gen.seed(seed);
            
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

        double uniform();
        double uniform(const double min, const double max);
        
        int genSampleSize(const double, const double, const double, const double);
        int genSampleSize(const std::vector<double> &intervals, const std::vector<double> &weights);
        
        // normal distribution
        arma::Mat<double>
        normal(const double mean, const double var, const double n);
        std::vector<arma::Row<double> >
        normal(const arma::Mat<double> &means, const arma::Mat<double> &vars, const int n);
        std::vector<arma::Row<double> >
        normal(const arma::Mat<double> &means, const arma::Mat<double> &vars, const arma::Mat<double> &nobs);

        // multivariate normal distribution
        std::vector<arma::Row<double> >
        mvnorm(const arma::Mat<double> &means, const arma::Mat<double> &sigma, const int n);
        std::vector<arma::Row<double> >
        mvnorm(const arma::Mat<double> &means, const arma::Mat<double> &sigma, const arma::Mat<double> &nobs);
        

    private:
        int seed;
    };
    
}

#endif //SAMPP_RANDOMNUMBERGENERATOR_H
