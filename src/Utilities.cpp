//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <iostream>

#include "Utilities.h"
#include "gsl/gsl_rng.h"
#include "gsl/gsl_randist.h"
#include "gsl/gsl_statistics.h"
#include "gsl/gsl_vector.h"
#include "gsl/gsl_matrix.h"


template<typename T>
std::vector<T> flatten(const std::vector<std::vector<T>> &orig){
	std::vector<T> ret;
	for(const auto &v: orig)
		ret.insert(ret.end(), v.begin(), v.end());
	return ret;
}


double mean(const std::vector<double>& v) {
	return gsl_stats_mean(v.data(), 1, v.size());
	return std::accumulate(v.begin(), v.end(), 0.) / v.size();
}

double sd(const std::vector<double>& v) {
    return gsl_stats_sd(v.data(), 1, v.size());
}

double cor(const std::vector<std::vector<double>>& dt, int i, int j){
	return gsl_stats_correlation(dt[i].data(), 1, dt[j].data(), 1, dt[i].size());
}


std::vector<std::vector<double> >
mvnorm_rng(){

	int ng = 1;
	int nd = 3;
	int n = 10000;

	std::vector<std::vector<double> > ran_vector;
	for (int i = 0; i < ng * nd; ++i) {
		ran_vector.push_back(std::vector<double>(n));
	}

	std::vector<double> means = {10, 20, 30};
	std::vector<std::vector<double >> vars = {{1, .5, 0}, {.5, 1, 0}, {0, 0, 1}};

	gsl_vector *mu = gsl_vector_alloc(means.size());
	gsl_matrix *sigma = gsl_matrix_alloc(means.size(), means.size());
	gsl_vector *res = gsl_vector_alloc(means.size());

	mu->data = means.data();
	sigma->data = flatten(vars).data();

	const gsl_rng_type *T;
	gsl_rng *r;

	gsl_rng_env_setup();

	T = gsl_rng_default;
	r = gsl_rng_alloc(T);


	// This for sure can be improved
	for (int i = 0; i < n; ++i) {
		gsl_ran_multivariate_gaussian(r, mu, sigma, res);
		for (int g = 0; g < ng; ++g) {
			for (int d = 0; d < nd; ++d) {
				int inx = g * nd + d;
				ran_vector[inx][i] = gsl_vector_get(res, d);
			}
		}
	}



	std::cout << std::endl;

	return ran_vector;

}


std::vector<std::vector<double> > 
norm_rng(){
	std::vector<std::vector<double> > ran_vector;

	double means[6] = {10, 20, 30, 40, 50, 60};
	double vars[6] = {1, 2, 3, 4, 5, 6};


	const gsl_rng_type *T;
	gsl_rng *r;

	int ng = 2;
	int nd = 3;
	int n = 10000;

	gsl_rng_env_setup();

	T = gsl_rng_default;
	r = gsl_rng_alloc(T);


    for (int g = 0; g < ng; ++g) {
        for (int d = 0; d < nd; ++d) {
        	int inx = g * nd + d;
        	std::cout << inx << ',';
            std::vector<double> _d_meas(n);
            for (int i = 0; i < n; ++i) {
                _d_meas[i] = means[inx] + gsl_ran_gaussian(r, vars[inx]);
            }
            ran_vector.push_back(_d_meas);
        }
    }

    std::cout << std::endl;

	return ran_vector;

}

std::vector<std::vector<double> >
gen_norm_rng(){
	std::vector<std::vector<double> > ran_vector;

	double means[6] = {10, 20, 30, 40, 50, 60};
	double vars[6] = {1, 2, 3, 4, 5, 6};


	const gsl_rng_type *T;
	gsl_rng *r;

	int ng = 2;
	int nd = 3;
	int n = 10000;

	gsl_rng_env_setup();

	T = gsl_rng_default;
	r = gsl_rng_alloc(T);


	for (int g = 0; g < ng; ++g) {
		for (int d = 0; d < nd; ++d) {
			int inx = g * nd + d;
			std::cout << inx << ',';
			std::vector<double> _d_meas(n);
			std::generate(_d_meas.begin(), _d_meas.end(), [r, vars, inx]() { return gsl_ran_gaussian(r, vars[inx]); } );
			ran_vector.push_back(_d_meas);
		}
	}

	std::cout << std::endl;

	return ran_vector;

}