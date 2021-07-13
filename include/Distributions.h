//===-- Distributions.h - Implementation of Some Utility Functions --------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-05-29.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the decelerations of some utility function which are
/// mainly dealing with the randomness and distribution setup.
///
//===----------------------------------------------------------------------===//
///
/// @defgroup DistributionBuilders Machinery of Building a Distribution Object
///

#ifndef SAMPP_UTILITIES_H
#define SAMPP_UTILITIES_H

#include "effolkronium/random.hpp"
#include "sam.h"

#include "baaraan/dists/mvnorm_distribution.h"
#include "baaraan/dists/truncated_normal_distribution.h"
#include "baaraan/dists/truncated_mvnorm_distribution.h"

namespace sam {

// A list of every supported univariate distributions
const std::map<std::string, std::vector<std::string>> univariate_dists = {
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
  {"truncated_normal_distribution", {"mean", "stddev", "min", "max"}}
};

// A list of every supported multivariate distribution
const std::map<std::string, std::vector<std::string>> multivariate_dists {
  // Multivariate Distros
  {"mvnorm_distribution", {"means", "sigma"}},
  {"truncated_mvnorm_distribution", {"means", "sigma", "lowers", "uppers"}}
};

}

using Random = effolkronium::random_static;

/** @name Distributions' Wrapper
 *
 *  These wrap the Univariate and Multivariate distributions to a function with a given,
 *  ie. std::mt19937, generator. This
 *
 *  @ingroup DistributionBuilders
 */
///@{
using Generator = std::mt19937;
using UnivariateDistribution = std::function<float(Generator &)>;
using MultivariateDistribution = std::function<arma::Mat<float>(Generator &)>;
///@}

/// Univariate Distribution's Constructor
UnivariateDistribution makeUnivariateDistribution(json const &j);

/// Multivariate Distribution's Constructor
MultivariateDistribution makeMultivariateDistribution(json const &j);

/** @name Abstract Distributions' Builders
 *
 *  These are being used by the `generate_*_distribution_factory` functions to generate
 *  an object of UnivariateDistribution or MultivariateDistribution
 *
 *  @ingroup DistributionBuilders
 */
///@{
/// Abstract builder of the univariate distributions
template <class DistributionType, class... Parameters>
UnivariateDistribution make_univariate_distribution_impl(json const &j, Parameters... parameters) {
  return DistributionType{j.at(parameters)...};
}

/// Abstract builder of the multivariate distributions
template <class DistributionType, class... Parameters>
MultivariateDistribution
make_multivariate_distribution_impl(json const &j, Parameters... parameters) {
  return DistributionType{j.at(parameters)...};
}
///@}


/// @brief Fills the matrix with values drawn from the given distribution
///
/// @note At least one of the optionals should have values!
static arma::Mat<float> fillMatrix(std::optional<std::vector<UnivariateDistribution>> &dists,
                            std::optional<MultivariateDistribution> &mdist,
                            int n_rows, int n_cols) {
  
  arma::Mat<float> data(n_rows, n_cols);
  
  if (mdist) {
    // Multivariate Distributions
    // Filling by columns because MultiDist returns a column of results
    data.each_col([&](arma::Col<float> &v) { v = Random::get(mdist.value()); });
  } else {
    if (dists) {
      // Set of Univariate Distributions
      // Filling by rows because each row has its own distribution now
      data.each_row([&, i = 0](arma::Row<float> &v) mutable {
        v.imbue([&]() { return Random::get(dists.value()[i]); });
        i++;
      });
    }
  }
  
  return data;
}


/// @brief Returns a vector of values based on the content of the JSON.
/// 
/// This is a very simplified version of Parameter<T>, when it can only handle
/// array and fixed values, but not distributions. It does the job, but I want
/// to remove it and replace whatever relies on it with Parameter<T>. However,
/// I faced some header collisions and I gave up!
///
/// @see Parameter<T>
template <typename T = float>
std::vector<T> get_expr_setup_params(json const &j, size_t size) {
  
  switch (j.type()) {
    case nlohmann::detail::value_t::array: {
      if (j.size() != size) {
        throw std::length_error("Array size does not match with the size\
                                of the experiment.\n");
      }
      return j.get<std::vector<T>>();
    }
      
    case nlohmann::detail::value_t::number_integer:
    case nlohmann::detail::value_t::number_unsigned:
    case nlohmann::detail::value_t::number_float:
      return std::vector<T>(size, j.get<T>());
      
    case nlohmann::detail::value_t::null:
    default:
      throw std::invalid_argument("Missing parameter.\n");
      
  }
}

/** @name Covariance matrix constructors
 *
 * These are handy functions for constructing a covariance matrix based on
 * users' input. For instance, they make it easy to setup a covariance with
 * fixed, or an array of standard deviations.
 *
 */
///@{
arma::Mat<float> constructCovMatrix(const arma::Row<float> &stddevs,
                                     const arma::Row<float> &covs,
                                     int n);

arma::Mat<float> constructCovMatrix(const arma::Row<float> &stddevs,
                                     float cov,
                                     int n);

arma::Mat<float> constructCovMatrix(float stddev,
                                     float cov,
                                     int n);
///@}


namespace nlohmann {

template <typename T> struct adl_serializer<arma::Row<T>> {

  static void to_json(json &j, const arma::Row<T> &row) {
    j = arma::conv_to<std::vector<T>>::from(row);
  }

  static void from_json(const json &j, arma::Row<T> &row) {
    row = arma::Row<T>(j.get<std::vector<T>>());
  }
};

template <typename T> struct adl_serializer<arma::Col<T>> {

  static void to_json(json &j, const arma::Col<T> &col) {
    for (int i{0}; i < col.n_rows; ++i) {
      j.push_back(arma::conv_to<std::vector<T>>::from(col.row(i)));
    }
  }

  static void from_json(const json &j, arma::Col<T> &col) {
    col = arma::Col<T>(j.size());
    for (int i{0}; i < j.size(); ++i) {
      assert(j[i].size() == 1);
      col.at(i) = j[i].get<std::vector<T>>()[0];
    }
  }
};

template <typename T> struct adl_serializer<arma::Mat<T>> {

  static void to_json(json &j, const arma::Mat<T> &mat) {

    for (int i{0}; i < mat.n_rows; ++i) {
      j.push_back(arma::conv_to<std::vector<T>>::from(mat.row(i)));
    }
  }

  static void from_json(const json &j, arma::Mat<T> &mat) {

    auto n_rows = j.size();
    auto n_cols = j[0].size();
    for (int i{0}; i < n_rows; ++i) {
      assert(j[i].size() == n_cols);
    }

    mat = arma::Mat<T>(n_rows, n_cols);
    for (int i{0}; i < n_rows; ++i) {
      mat.row(i) = j[i].get<arma::Row<T>>();
    }
  }
};

template <typename T> struct adl_serializer<baaraan::mvnorm_distribution<T>> {

  static void to_json(json &j, const baaraan::mvnorm_distribution<T> &mdist) {

    j["dist"] = "mvnorm_distribution";
    j["means"] = arma::Row<float>(mdist.means().as_row());
    j["sigma"] = mdist.sigma();
  }

  static baaraan::mvnorm_distribution<T> from_json(const json &j) {

    arma::Row<float> means = j.at("means").get<arma::Row<float>>();
    arma::Mat<float> sigma;
    auto n_dims = means.n_elem;

    if (j.find("sigma") != j.end()) {
      sigma = j.at("sigma").get<arma::Mat<float>>();
      // TODO: Check for sigma dimension
    } else {
      if (j.find("stddevs") == j.end() || j.find("covs") == j.end()) {
        throw std::invalid_argument(
            "Either `sigma` or `covs` and `stddevs` have to be given.");
      }
      arma::Row<float> stddevs = get_expr_setup_params(j.at("stddevs"), n_dims);
      arma::Row<float> covs =
      get_expr_setup_params(j.at("covs"), n_dims * (n_dims - 1) / 2);
      sigma = constructCovMatrix(stddevs, covs, n_dims);
    }

    return baaraan::mvnorm_distribution<T>{means.t(), sigma};
  }
};

template <typename T>
struct adl_serializer<baaraan::truncated_mvnorm_distribution<T>> {

  static void to_json(json &j,
                      const baaraan::truncated_mvnorm_distribution<T> &mdist) {

    j["dist"] = "truncated_mvnorm_distribution";
    j["means"] = arma::Row<float>(mdist.means().as_row());
    j["sigma"] = mdist.sigma();
    j["lowers"] = arma::Row<float>(mdist.lowers().as_row());
    j["uppers"] = arma::Row<float>(mdist.uppers().as_row());
  }

  static baaraan::truncated_mvnorm_distribution<T> from_json(const json &j) {

    auto means = j.at("means").get<arma::Row<float>>();
    auto n_dims = means.n_elem;

    arma::Mat<T> sigma;
    arma::Mat<T> lowers;
    arma::Mat<T> uppers;

    if (j.find("sigma") != j.end()) {
      sigma = j.at("sigma").get<arma::Mat<float>>();
      if (sigma.n_elem != n_dims * n_dims) {
        throw std::domain_error("`sigma` doesn't have the correct size.");
      }
    } else {
      if (j.find("stddevs") == j.end() || j.find("covs") == j.end()) {
        throw std::invalid_argument(
            "Either `sigma` or `covs` and `stddevs` have to be given.");
      }

      arma::Row<float> stddevs = get_expr_setup_params(j.at("stddevs"), n_dims);
      arma::Row<float> covs =
      get_expr_setup_params(j.at("covs"), n_dims * (n_dims - 1) / 2);
      sigma = constructCovMatrix(stddevs, covs, n_dims);
    }

    if (j.find("lowers") == j.end() || j.find("uppers") == j.end()) {
      throw std::invalid_argument("lower or upper boundaries are missing.");
    }

    lowers = get_expr_setup_params(j.at("lowers"), n_dims);
    uppers = get_expr_setup_params(j.at("uppers"), n_dims);

    return baaraan::truncated_mvnorm_distribution<T>{means.t(), sigma, lowers,
                                                     uppers};
  }
};

} // namespace nlohmann

#endif // SAMPP_UTILITIES_H
