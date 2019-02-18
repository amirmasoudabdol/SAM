//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_RANDOMNUMBERGENERATOR_H
#define SAMPP_RANDOMNUMBERGENERATOR_H

#include <vector>
#include <string>
#include "gsl/gsl_rng.h"
#include "gsl/gsl_randist.h"


class RandomNumberGenerator {

public:

    RandomNumberGenerator(int seed, bool is_correlated) :
        _seed(seed), _is_correlated(is_correlated) {
            
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
    double uniform(double min, double max);
    
    int genSampleSize(double, double, double, double);

    std::vector<double> normal(double mean, double sd, int n);
    std::vector<std::vector<double> > normal(std::vector<double>& means, std::vector<double>& sds, int n);
    std::vector<std::vector<double>> normal(std::vector<double>& means, std::vector<double>& sds, std::vector<int> nobs);

    std::vector<double> mvnorm(std::vector<double>& means, std::vector<std::vector<double> >& sigma);
    std::vector<std::vector<double> > mvnorm(std::vector<double>& means, std::vector<std::vector<double> >& sigma, int n);
    
    void mvnorm_n(gsl_vector* means, gsl_matrix* sigma, gsl_matrix* ran_values);

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
    gsl_matrix *_mvnorm_nrows_ncols;

    const gsl_rng_type *T;
    gsl_rng *rng_stream;
    int _seed;
    bool _is_correlated;
    bool _is_gsl_containers_initialized = false;
};

#endif //SAMPP_RANDOMNUMBERGENERATOR_H
