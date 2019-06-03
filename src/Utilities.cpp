//
// Created by Amir Masoud Abdol on 2019-05-29.
//

#include "Utilities.h"

using Generator = std::mt19937;


Distribution make_distribution(json const &j) {
    auto const &distributionName = j.at("dist");

    
    // Both piecewise distributions need be handled differently because they accept
    // list initilizers rather than a container.
    if (distributionName == "piecewise_linear_distribution") {
        return std::piecewise_linear_distribution<>(
                    std::piecewise_linear_distribution<>::param_type(j.at("intervals").begin(),
                                                                     j.at("intervals").end(),
                                                                     j.at("densities").begin()));
    }
    if (distributionName == "piecewise_constant_distribution") {
        return std::piecewise_constant_distribution<>(
                    std::piecewise_constant_distribution<>::param_type(j.at("intervals").begin(),
                                                                       j.at("intervals").end(),
                                                                       j.at("densities").begin()));
    }
    
    // Special case for Bernulli Distribution because it's the only one that it's not
    // templated
    if (distributionName == "bernoulli_distribution") {
        return std::bernoulli_distribution(
                    std::bernoulli_distribution::param_type(j.at("p")));
    }

/**
 A macro generating different functions calls based on the given distribution.

 @param name_ The distribution name
 @param type_ The distribution type
 @param ... Paramters of the distribution, according to their representation in
 the standard library
 @return A function call to the make_distribution_impl that returns a distribution
 class.
 */
#define generate_distribution_factory(name_, type_, ...) \
if(distributionName == #name_) return make_distribution_impl<std::name_<type_>>(j, ## __VA_ARGS__);

    // Continious Distributions
    generate_distribution_factory(uniform_int_distribution, double, "a", "b");
    generate_distribution_factory(uniform_real_distribution, double, "a", "b");
    generate_distribution_factory(binomial_distribution, double, "p", "t");
    generate_distribution_factory(exponential_distribution, double, "lambda");
    generate_distribution_factory(gamma_distribution, double, "alpha", "beta");
    generate_distribution_factory(weibull_distribution, double, "a", "b");
    generate_distribution_factory(extreme_value_distribution, double, "a", "b");
    generate_distribution_factory(normal_distribution, double, "mean", "stddev");
    generate_distribution_factory(lognormal_distribution, double, "m", "s");
    generate_distribution_factory(chi_squared_distribution, double, "n");
    generate_distribution_factory(cauchy_distribution, double, "a", "b");
    generate_distribution_factory(fisher_f_distribution, double, "m", "n");
    generate_distribution_factory(student_t_distribution, double, "n");

    // Discrete distributions
    generate_distribution_factory(negative_binomial_distribution, int, "p", "k");
    generate_distribution_factory(geometric_distribution, int, "p");
    generate_distribution_factory(poisson_distribution, int, "mean");
    generate_distribution_factory(discrete_distribution, int, "probabilities");

#undef generate_distribution_factory

    throw std::runtime_error{&"Unknown distribution " [ distributionName]};
}
