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
double sd(const std::vector<double> &);
double cor(const std::vector<std::vector<double>>&, int, int);

std::vector<std::vector<double> > norm_rng();
std::vector<std::vector<double> > gen_norm_rng();
std::vector<std::vector<double> > mvnorm_rng();
//std::vector<std::vector<double> > mvnorm(std::vector<double>, std::vector<std::vector<double> >, int);



#endif //SAMPP_UTILITIES_H
