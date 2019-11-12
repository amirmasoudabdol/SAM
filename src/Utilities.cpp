//
// Created by Amir Masoud Abdol on 2019-05-29.
//

#include "Utilities.h"
#include "mvrandom.hpp"
#include "utils/truncated_normal_distribution.h"

using Generator = std::mt19937;

namespace sam {

    /// A dataset of each distributions with their parameters name.
    const std::map<std::string, std::vector<std::string>> disto_params_names =
    {
        {"uniform_int_distribution", {"a", "b"}},
        {"uniform_real_distribution", {"a", "b"}},
        {"binomial_distribution", {"p", "t"}},
        {"exponential_distribution", {"lambda"}},
        {"gamma_distribution", {"alpha", "beta"}},
        {"weibull_distribution", {"a", "b"}},
        {"extreme_value_distribution", {"a", "b"}},
        {"normal_distribution", {"mean", "stddev"}},
        {"lognormal_distribution", {"m", "s"}},
        {"chi_squared_distribution", {"n"}},
        {"cauchy_distribution", {"a", "b"}},
        {"fisher_f_distribution", {"m", "n"}},
        {"student_t_distribution", {"n"}},
        {"negative_binomial_distribution", {"p", "k"}},
        {"geometric_distribution", {"p"}},
        {"poisson_distribution", {"mean"}},
        {"discrete_distribution", {"probabilities"}},
        {"piecewise_linear_distribution", {"intervals", "densities"}},
        {"piecewise_constant_distribution", {"intervals", "densities"}},
        {"bernoulli_distribution", {"p"}},
        {"truncated_normal_distribution", {"mean", "stddev", "min", "max"}},

        // Multivariate Distros
        {"mvnorm_distribution", {"means", "covs"}},
        {"truncated_mvnorm_distribution", {"means", "covs", "lowers", "uppers"}}
    };

}

arma::Mat<double>
constructCovMatrix(const arma::Row<double> &vars, const double cov, int n) {
    arma::Mat<double> cov_matrix(n, n);
    
    cov_matrix.fill(cov);
    cov_matrix.diag() = vars;
    
    return cov_matrix;
}

arma::Mat<double>
constructCovMatrix(const arma::Row<double> &vars, const arma::Row<double> &covs, int n) {
    using namespace arma;

    assert(covs.size() == n * (n - 1) / 2);

    mat temp(n, n);
    temp.fill(-1);

    mat L = trimatl(temp, -1);

    mat::row_iterator row_it     = L.begin_row(1);  // start of rowumn 1
    mat::row_iterator row_it_end = L.end_row(n-1);    //   end of rowumn 3

    for(int i = 0; row_it != row_it_end; ++row_it)
        if ((*row_it) == -1)
            (*row_it) = covs(i++);

    mat sigma = symmatl(L);
    sigma.diag() = vars;

    return sigma;
}

arma::Mat<double>
constructCovMatrix(const double var, const double cov, const int n) {
    arma::Row<double> vars(n);
    vars.fill(var);
    return constructCovMatrix(vars, cov, n);
}


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
    
    // Custom Distributions
    if (distributionName == "truncated_normal_distribution") {
        return truncated_normal_distribution<>(
                    truncated_normal_distribution<>::param_type(j.at("mean"),
                                                                   j.at("stddev"),
                                                                   j.at("min"),
                                                                   j.at("max")));
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


MultivariateDistribution make_multivariate_distribution(json const &j) {

    auto const &distributionName = j.at("dist");

/**
 A macro generating different functions calls based on the given distribution. The main difference here is
 the fact that the `std::function` has the form of `std::function<arma::mat<double>(Generator &)>`.

 @param name_ The distribution name
 @param type_ The distribution type
 @param ... Paramters of the distribution, according to their representation in
 the standard library
 @return A function call to the make_distribution_impl that returns a distribution
 class.
 */
#define generate_multivariate_distribution_factory(name_, type_, ...) \
if(distributionName == #name_) return make_multivariate_distribution_impl<name_<type_>>(j, ## __VA_ARGS__);

    generate_multivariate_distribution_factory(mvrandom::mvnorm_distribution, double, "means", "covs");
    generate_multivariate_distribution_factory(mvrandom::truncated_mvnorm_distribution, double, "means", "covs", "lowers", "uppers");

#undef generate_multivariate_distribution_factory

    throw std::runtime_error{&"Unknown distribution " [ distributionName]};
}
