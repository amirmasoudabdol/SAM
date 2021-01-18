//===-- Utilities.cpp - Implementation of Utility Functions ---------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-05-29.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the implementation of some utility functions mainly
/// concerning the distribution setup, and everything around it, e.g. JSON
/// serializer, and object constructions.
///
//===----------------------------------------------------------------------===//

#include "Distributions.h"

using namespace sam;

using Generator = std::mt19937;

/// @brief      Makes a univariate distribution.
///
/// When I implemented this, my goal was to write as little code as possible. 
/// So, this function utilizes a macro and generates a function call for based 
/// on the distribution specification. The `generate_univariate_distribution_factory`
/// marco handles this part. Basically, besides a few edge cases, the macro 
/// automatically generates a call to make_univariate_distribution_impl() with 
/// the given parameters in their right place.
///
/// @param      j     The distribution specification
///
/// @return     The univariate distribution
UnivariateDistribution makeUnivariateDistribution(json const &j) {
  auto const &distributionName = j.at("dist");

  // Both piecewise distributions need be handled differently because they
  // accept list initializers rather than a container.
  if (distributionName == "piecewise_linear_distribution") {
    return std::piecewise_linear_distribution<>(
        std::piecewise_linear_distribution<>::param_type(
            j.at("intervals").begin(), j.at("intervals").end(),
            j.at("densities").begin()));
  }
  if (distributionName == "piecewise_constant_distribution") {
    return std::piecewise_constant_distribution<>(
        std::piecewise_constant_distribution<>::param_type(
            j.at("intervals").begin(), j.at("intervals").end(),
            j.at("densities").begin()));
  }

  // Special case for Bernoulli Distribution because it's the only one that it's
  // not templated
  if (distributionName == "bernoulli_distribution") {
    return std::bernoulli_distribution(
        std::bernoulli_distribution::param_type(j.at("p")));
  }

  // Custom Distributions
  if (distributionName == "truncated_normal_distribution") {
    return baaraan::truncated_normal_distribution<>(
        baaraan::truncated_normal_distribution<>::param_type(
            j.at("mean"), j.at("stddev"), j.at("lower"), j.at("upper")));
  }

///
/// A macro generating different functions calls based on the given
/// distribution.
///
/// @param      name_  The distribution name
/// @param      type_  The distribution type
/// @param      ...    Parameters of the distribution, according to their
///                    representation in the standard library
/// @return     A function call to the make_univariate_distribution_impl that returns a
///             distribution class.
///
#define generate_univariate_distribution_factory(name_, type_, ...)            \
  if (distributionName == #name_)                                              \
    return make_univariate_distribution_impl<std::name_<type_>>(j, ##__VA_ARGS__);

  // Continuous Distributions
  generate_univariate_distribution_factory(uniform_int_distribution, int, "a", "b")
  generate_univariate_distribution_factory(uniform_real_distribution, double, "a", "b")
  generate_univariate_distribution_factory(binomial_distribution, int, "p", "t")
  generate_univariate_distribution_factory(exponential_distribution, double, "lambda")
  generate_univariate_distribution_factory(gamma_distribution, double, "alpha", "beta")
  generate_univariate_distribution_factory(weibull_distribution, double, "a", "b")
  generate_univariate_distribution_factory(extreme_value_distribution, double, "a", "b")
  generate_univariate_distribution_factory(normal_distribution, double, "mean", "stddev")
  generate_univariate_distribution_factory(lognormal_distribution, double, "m", "s")
  generate_univariate_distribution_factory(chi_squared_distribution, double, "n")
  generate_univariate_distribution_factory(cauchy_distribution, double, "a", "b")
  generate_univariate_distribution_factory(fisher_f_distribution, double, "m", "n")
  generate_univariate_distribution_factory(student_t_distribution, double, "n")

  // Discrete distributions
  generate_univariate_distribution_factory(negative_binomial_distribution, int, "p", "k")
  generate_univariate_distribution_factory(geometric_distribution, int, "p")
  generate_univariate_distribution_factory(poisson_distribution, int, "mean")
  generate_univariate_distribution_factory(discrete_distribution, int, "probabilities")

#undef generate_univariate_distribution_factory

  //    throw std::runtime_error{&"Unknown distribution " [ distributionName]};
  throw std::runtime_error{"Unknown distribution"};
}

///
/// @brief      Makes a multivariate distribution.
///
/// @see        makeUnivariateDistribution()
///
/// @param      j     The distribution specification
///
/// @return     The multivariate distribution.
///
MultivariateDistribution makeMultivariateDistribution(json const &j) {

  auto const &distributionName = j.at("dist");

  if (distributionName == "mvnorm_distribution") {
    return j.get<baaraan::mvnorm_distribution<double>>();
  }

  if (distributionName == "truncated_mvnorm_distribution") {
    return j.get<baaraan::truncated_mvnorm_distribution<double>>();
  }

///
/// A macro generating different functions calls based on the given
/// distribution. The main difference here is the fact that the `std::function`
/// has the form of `std::function<arma::mat<double>(Generator &)>`.
///
/// @param      name_  The distribution name
/// @param      type_  The distribution type
/// @param      ...    Parameters of the distribution, according to their
///                    representation in the standard library
/// @return     A function call to the make_multivariate_distribution_impl that returns a
///             distribution class.
///
#define generate_multivariate_distribution_factory(name_, type_, ...)          \
  if (distributionName == #name_)                                              \
    return make_multivariate_distribution_impl<name_<type_>>(j, ##__VA_ARGS__);

  generate_multivariate_distribution_factory(baaraan::mvnorm_distribution,
                                             double, "means", "covs")
  generate_multivariate_distribution_factory(
      baaraan::truncated_mvnorm_distribution, double, "means", "covs", "lowers",
      "uppers")

#undef generate_multivariate_distribution_factory

  throw std::runtime_error{"Unknown distribution "};
}


/// @brief      Constructs a covariance matrix using fixed cov and stddev values
///
/// @param[in]  stddev  The stddev
/// @param[in]  cov     The cov
/// @param[in]  n       The size of the diagonal matrix
///
/// @return     The covariance matrix
arma::Mat<double> constructCovMatrix(const double stddev,
                                     const double cov,
                                     const int n) {
  arma::Row<double> stddevs(n);
  stddevs.fill(stddev);
  return constructCovMatrix(stddevs, cov, n);
}

/// @brief      Constructs a covariance matrix using a vector of covs and stddevs
///
/// @param[in]  stddevs  The stddevs
/// @param[in]  covs     The covs
/// @param[in]  n        The size of the diagonal matrix
///
/// @return     The covariance matrix
arma::Mat<double> constructCovMatrix(const arma::Row<double> &stddevs,
                                     const arma::Row<double> &covs,
                                     int n) {
  using namespace arma;

  assert(covs.size() == n * (n - 1) / 2);

  arma::rowvec vars = arma::pow(stddevs, 2);

  mat temp(n, n);
  temp.fill(-1);

  mat L = trimatl(temp, -1);

  mat::row_iterator row_it = L.begin_row(1);       // start of rownum 1
  mat::row_iterator row_it_end = L.end_row(n - 1); //   end of rownum 3

  for (int i = 0; row_it != row_it_end; ++row_it)
    if ((*row_it) == -1)
      (*row_it) = covs(i++);

  mat sigma = symmatl(L);
  sigma.diag() = vars;

  return sigma;
}

/// @brief      Constructs a covariance matrix with fixed covariance, and variable stddevs given
///
/// @param[in]  stddevs  The stddevs
/// @param[in]  cov      The cov
/// @param[in]  n        The size of the diagonal matrix
///
/// @return     The covariance matrix
arma::Mat<double> constructCovMatrix(const arma::Row<double> &stddevs,
                                     const double cov,
                                     int n) {
  arma::Mat<double> cov_matrix(n, n);
  
  cov_matrix.fill(cov);
  cov_matrix.diag() = arma::pow(stddevs, 2);
  
  return cov_matrix;
}

