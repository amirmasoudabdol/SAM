//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include "RandomNumberGenerator.h"

#include <numeric>
#include <algorithm>
#include <RandomNumberGenerator.h>
#include "Utilities.h"

#include "gsl/gsl_rng.h"

//std::vector<double>
//RandomNumberGenerator::normal(double mean, double sd, int n){
//    std::vector<double> rn(n);
//    std::generate(rn.begin(), rn.end(), [this, mean, sd]() {return mean + gsl_ran_gaussian(this->rng_stream, sd); } );
//    return rn;
//}

arma::Row<double>
RandomNumberGenerator::normal(const double &mean, const double &sd, const double &n){
    return sd * arma::randn<arma::Row<double>>(n) + mean;
}

//std::vector<std::vector<double> >
//RandomNumberGenerator::normal(std::vector<double>& means, std::vector<double>& sds, int n) {
//    std::vector<std::vector<double>> rns;
//
//    for (int i = 0; i < means.size(); ++i){
//        rns.push_back(normal(means[i], sds[i], n));
//    }
//
//    return rns;
//}

// arma
std::vector<arma::Row<double> >
RandomNumberGenerator::normal(arma::Row<double>& means, arma::Row<double>& sds, int n) {

    std::vector<arma::Row<double>> rns;

    for (int i = 0; i < means.size(); ++i){
        rns.push_back(normal(means[i], sds[i], n));
    }

    return rns;
}
//
//std::vector<std::vector<double>>
//RandomNumberGenerator::normal(std::vector<double>& means, std::vector<double>& sds, std::vector<int> nobs){
//    std::vector<std::vector<double>> rns;
//
//    for (int i = 0; i < means.size(); ++i){
//        rns.push_back(normal(means[i], sds[i], nobs[i]));
//    }
//
//    return rns;
//}

// arma 
std::vector<arma::Row<double>>
RandomNumberGenerator::normal(arma::Row<double>& means, arma::Row<double>& sds, arma::Row<int> nobs){
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
RandomNumberGenerator::mvnorm(arma::Row<double>& means, arma::Mat<double>& sigma, int n){
    arma::mat rans_mat = arma::mvnrnd(means.t(), sigma, n);
    
    std::vector<arma::Row<double> > rans;
    for (int i = 0 ; i < means.size(); i ++) {
        rans.push_back(rans_mat.row(i));
    }
    
    return rans;
}

// arma
std::vector<arma::Row<double> >
RandomNumberGenerator::mvnorm(arma::Row<double>& means, arma::Mat<double>& sigma, arma::Row<int>& nobs){
    
    // TODO: This can be optimized by some STL algorithm as well
    arma::mat rans_mat = arma::mvnrnd(means.t(), sigma, arma::max(nobs));
    
    std::vector<arma::Row<double> > rans;
    for (int i = 0 ; i < means.size(); i ++) {
        rans.push_back(rans_mat.row(i));
    }
    
    return rans;
}

//std::vector<double>
//RandomNumberGenerator::mvnorm(std::vector<double>& means, std::vector<std::vector<double> >& sigma) {
//
//    std::vector<double> rn(means.size());
//
//
//    // TODO: This can be moved to the Constructor somewhere!
//    // FIXME: This was the source of error before due to non-initialized variables
//    if (!_is_gsl_containers_initialized){
//        _size = means.size();
//        allocGSLContainers();
//    } // TODO: Check the case that we suddenly decide to change the size of means;
//
//
//    _mu->data = means.data();
//    _sigma->data = flatten(sigma).data();
//
//    gsl_ran_multivariate_gaussian(rng_stream, _mu, _sigma, _mvnorm_row);
//
//    for (int i = 0; i < means.size(); ++i) {
//        rn[i] = gsl_vector_get(_mvnorm_row, i);
//    }
//
//    return rn;
//}
//
//std::vector<std::vector<double> >
//RandomNumberGenerator::mvnorm(std::vector<double>& means, std::vector<std::vector<double> >& sigma, int n) {
//
//    std::vector<std::vector<double> > rns;
//    for (int m = 0; m < means.size(); ++m){
//        rns.push_back(std::vector<double>(n));
//    }
//
//    for (int i = 0; i < n; ++i){
//        std::vector<double> v = mvnorm(means, sigma);
//        for (int m = 0; m < means.size(); ++m){
//            rns[m][i] = v[m];
//        }
//    }
//
//    return rns;
//}

double RandomNumberGenerator::uniform() {
    return gsl_ran_flat(rng_stream, 0, 1);
}

double RandomNumberGenerator::uniform(double min, double max){
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

int RandomNumberGenerator::genSampleSize(double p, double a, double b, double c){
    if (gsl_ran_bernoulli(rng_stream, p)) {
        return static_cast<int>(gsl_ran_flat(rng_stream, a, b));
    }else{
        return static_cast<int>(gsl_ran_flat(rng_stream, b, c));
    }
}

// void RandomNumberGenerator::mvnorm_n(gsl_vector *means, gsl_matrix *sigma, gsl_matrix *ran_values) {
    
//     if (!_is_gsl_containers_initialized){
// //        _size = means.size();
//         _size = means->size;
//         allocGSLContainers();
//     } // TODO: Check the case that we suddenly decide to change the size of means;

    
//     gsl_vector* tmp_vector = gsl_vector_alloc(means->size);
    
//     for (int i = 0; i < ran_values->size2; i++) {
//         gsl_ran_multivariate_gaussian(rng_stream, means, sigma, tmp_vector);
//         gsl_matrix_set_col(ran_values, i, tmp_vector);
//     }
    
//     // for (int i = 0; i < ran_values->size1; i++) {
//     //     for (int j = 0; j < ran_values->size2; j++) {
//     //         std::cout << i << ", " << j << ": " << gsl_matrix_get(ran_values, i, j) << std::endl;
//     //     }
//     //     std::cout << "\n";
//     // }
    
//     gsl_vector_free(tmp_vector);
// }

