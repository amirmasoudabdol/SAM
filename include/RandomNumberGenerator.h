//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_RANDOMNUMBERGENERATOR_H
#define SAMPP_RANDOMNUMBERGENERATOR_H

#include <vector>
#include <string>
#include <armadillo>
#include "gsl/gsl_rng.h"
#include "gsl/gsl_randist.h"
#include "Utilities.h"


class RandomNumberGenerator {

public:

    RandomNumberGenerator(int seed, bool is_correlated) :
        _seed(seed), _is_correlated(is_correlated) {

        // TODO: Need to adopt this to arma setup
            
        gsl_rng_env_setup();
        T = gsl_rng_default;
        rng_stream = gsl_rng_alloc(T);
        if (seed != -1)
            gsl_rng_set(rng_stream, _seed);
        else
            // TODO: This is not a good idea, since `gsl_rng_default_seed` is always 0!
            gsl_rng_set(rng_stream, gsl_rng_default_seed);
    };
    
    ~RandomNumberGenerator() {

        // I think I'm surely missing stuff here!
        if (_is_correlated && _is_gsl_containers_initialized){
            gsl_rng_free(rng_stream);
            gsl_vector_free(_mu);
            gsl_matrix_free(_sigma);
            gsl_vector_free(_mvnorm_row);
        }
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
    
    // arma
    arma::Row<double>
    normal(const double &mean, const double &sd, const double &n);

    // arma
    std::vector<arma::Row<double> >
    normal(const arma::Row<double> &means, const arma::Row<double> &sds, const int &n);
    std::vector<arma::Row<double> >
    normal(const arma::Row<double> &means, const arma::Row<double> &sds, const arma::Row<int> &nobs);

    // arma
    std::vector<arma::Row<double> >
    mvnorm(const arma::Row<double> &means, const arma::Mat<double> &sigma, const int &n);
    std::vector<arma::Row<double> >
    mvnorm(const arma::Row<double> &means, const arma::Mat<double> &sigma, const arma::Row<int> &nobs);
    
    void allocGSLContainers() {
        // TODO: I get the idea here but I think it's terrible, I need to change it!
        _mu = gsl_vector_alloc(_size);
        _sigma = gsl_matrix_alloc(_size, _size);
        _mvnorm_row = gsl_vector_alloc(_size);

        _is_gsl_containers_initialized = true;
    };


private:
    int _size = 0;
    gsl_vector *_mu;
    gsl_matrix *_sigma;
    gsl_vector *_mvnorm_row;

    const gsl_rng_type *T;
    gsl_rng *rng_stream;
    int _seed;
    bool _is_correlated;
    bool _is_gsl_containers_initialized = false;
};

#endif //SAMPP_RANDOMNUMBERGENERATOR_H
