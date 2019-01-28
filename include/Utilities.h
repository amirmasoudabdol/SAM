//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_UTILITIES_H
#define SAMPP_UTILITIES_H

#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>


double mean(const std::vector<double> &);
double standard_dv(const std::vector<double> &);
double variance(const std::vector<double> &);
double cor(const std::vector<std::vector<double>>&, int, int);

std::vector<std::vector<double> > norm_rng();
std::vector<std::vector<double> > gen_norm_rng();
std::vector<std::vector<double> > mvnorm_rng();
//std::vector<std::vector<double> > mvnorm(std::vector<double>, std::vector<std::vector<double> >, int);


std::pair<double, double> oneSampleTTest(double mean1, double sd1, int nobs1, double mean2, double sd2, int nobs2, bool equal_var = false);

std::pair<double, double> equal_var_ttest_denom(double v1, int n1, double v2, int n2);

double ttest_finish(double t, double df);

template<typename T>
std::vector<T> flatten(const std::vector<std::vector<T>> &orig);

#endif //SAMPP_UTILITIES_H
