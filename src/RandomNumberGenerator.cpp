//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include "RandomNumberGenerator.h"

#include <numeric>
#include <algorithm>
#include <RandomNumberGenerator.h>
#include "Utilities.h"

#include "gsl/gsl_rng.h"

std::vector<double> 
RandomNumberGenerator::normal(double mean, double sd, int n){
	std::vector<double> rn(n);
	std::generate(rn.begin(), rn.end(), [this, mean, sd]() {return mean + gsl_ran_gaussian(this->rng_stream, sd); } );
	return rn;
}

std::vector<std::vector<double> >
RandomNumberGenerator::normal(std::vector<double>& means, std::vector<double>& sds, int n) {
	std::vector<std::vector<double>> rns;

	for (int i = 0; i < means.size(); ++i){
		rns.push_back(normal(means[i], means[i], n));
	}

	return rns;
}

std::vector<double>
RandomNumberGenerator::mvnorm(std::vector<double>& means, std::vector<std::vector<double> >& sigma) {

	std::vector<double> rn(means.size());


	if (!_is_gsl_containers_initialized){
		_size = means.size();
		allocGSLContainers();
	} // TODO: Check the case that we suddenly decide to change the size of means;


	_mu->data = means.data();
	_sigma->data = flatten(sigma).data();

	gsl_ran_multivariate_gaussian(rng_stream, _mu, _sigma, _mvnorm_row);

	for (int i = 0; i < means.size(); ++i) {
		rn[i] = gsl_vector_get(_mvnorm_row, i);
	}

	return rn;
}

std::vector<std::vector<double> >
RandomNumberGenerator::mvnorm(std::vector<double>& means, std::vector<std::vector<double> >& sigma, int n) {

	std::vector<std::vector<double> > rns;
	for (int m = 0; m < means.size(); ++m){
		rns.push_back(std::vector<double>(n));
	}

	for (int i = 0; i < n; ++i){
		std::vector<double> v = mvnorm(means, sigma);
		for (int m = 0; m < means.size(); ++m){
			rns[m][i] = v[m];
		}
	}

	return rns;
}

double RandomNumberGenerator::uniform() {
    return gsl_rng_uniform(rng_stream);
}
