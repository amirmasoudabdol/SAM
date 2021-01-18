//===-- Distributions.h - Implementation of Some Utility Functions ------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-05-29.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the decelerations of some utility function which are
/// mainly dealing with the randomness and distributino setup.
///
//===----------------------------------------------------------------------===//

#ifndef SAMPP_UTILITIES_H
#define SAMPP_UTILITIES_H

#include "effolkronium/random.hpp"
#include "sam.h"
#include "baaraan.hpp"

namespace sam {

static const std::map<std::string, std::vector<std::string>> univariate_dists = {
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

// A list of every multivariate distribution
static const std::map<std::string, std::vector<std::string>> multivariate_dists {
  // Multivariate Distros
  {"mvnorm_distribution", {"means", "covs"}},
  {"truncated_mvnorm_distribution", {"means", "covs", "lowers", "uppers"}}
};

}



using Random = effolkronium::random_static;

using Generator = std::mt19937;
using UnivariateDistribution = std::function<double(Generator &)>;
using MultivariateDistribution = std::function<arma::mat(Generator &)>;

/// Univariate Distribution's Constructor
UnivariateDistribution makeUnivariateDistribution(json const &j);

/// Multivariate Distribution's Constructor
MultivariateDistribution makeMultivariateDistribution(json const &j);


/// @brief Fills the matrix with values drawn from the given distribution
///
/// @note At least one of the optionals should have values!
static arma::mat fillMatrix(std::optional<std::vector<UnivariateDistribution>> &dists,
                            std::optional<MultivariateDistribution> &mdist,
                            int n_rows, int n_cols) {
  
  arma::mat data(n_rows, n_cols);
  
  if (mdist) {
    // Multivariate Distributions
    // Filling by columns because MultiDist returns a column of results
    data.each_col([&](arma::vec &v) { v = Random::get(mdist.value()); });
  } else {
    if (dists) {
      // Set of Univariate Distributions
      // Filling by rows because each row has its own distribution now
      data.each_row([&, i = 0](arma::rowvec &v) mutable {
        v.imbue([&]() { return Random::get(dists.value()[i]); });
        i++;
      });
    }
  }
  
  return data;
}

template <typename T = double>
std::vector<T> get_expr_setup_params(json const &j, int const size) {
  
  switch (j.type()) {
    case nlohmann::detail::value_t::array: {
      if (j.size() != size)
        throw std::length_error("Array size does not match with the size\
                                of the experiment.\n");
      else
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

template <class DistributionType, class... Parameters>
UnivariateDistribution make_univariate_distribution_impl(json const &j, Parameters... parameters) {
  return DistributionType{j.at(parameters)...};
}

template <class DistributionType, class... Parameters>
MultivariateDistribution
make_multivariate_distribution_impl(json const &j, Parameters... parameters) {
  return DistributionType{j.at(parameters)...};
}

/** @name Constructing 
 *
 */
arma::Mat<double> constructCovMatrix(const arma::Row<double> &stddevs,
                                     const arma::Row<double> &covs,
                                     int n);

arma::Mat<double> constructCovMatrix(const arma::Row<double> &stddevs,
                                     const double cov,
                                     int n);

arma::Mat<double> constructCovMatrix(const double stddev,
                                     const double cov,
                                     const int n);


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
    for (int i{0}; i < n_rows; ++i)
      assert(j[i].size() == n_cols);

    mat = arma::Mat<T>(n_rows, n_cols);
    for (int i{0}; i < n_rows; ++i) {
      mat.row(i) = j[i].get<arma::Row<T>>();
    }
  }
};

template <typename T> struct adl_serializer<baaraan::mvnorm_distribution<T>> {

  static void to_json(json &j, const baaraan::mvnorm_distribution<T> &mdist) {

    j["dist"] = "mvnorm_distribution";
    j["means"] = arma::rowvec(mdist.means().as_row());
    j["sigma"] = mdist.sigma();
  }

  static baaraan::mvnorm_distribution<T> from_json(const json &j) {

    arma::Row<double> means = j.at("means").get<arma::Row<double>>();
    arma::Mat<double> sigma;
    auto n_dims = means.n_elem;

    if (j.find("sigma") != j.end()) {
      sigma = j.at("sigma").get<arma::Mat<double>>();
      // TODO: Check for sigma dimension
    } else {
      if (j.find("stddevs") == j.end() || j.find("covs") == j.end()) {
        std::invalid_argument(
            "Either `sigma` or `covs` and `stddevs` have to be given.");
      }
      arma::rowvec stddevs = get_expr_setup_params(j.at("stddevs"), n_dims);
      arma::rowvec covs =
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
    j["means"] = arma::rowvec(mdist.means().as_row());
    j["sigma"] = mdist.sigma();
    j["lowers"] = arma::rowvec(mdist.lowers().as_row());
    j["uppers"] = arma::rowvec(mdist.uppers().as_row());
  }

  static baaraan::truncated_mvnorm_distribution<T> from_json(const json &j) {

    auto means = j.at("means").get<arma::Row<double>>();
    auto n_dims = means.n_elem;

    arma::Mat<T> sigma;
    arma::Mat<T> lowers, uppers;

    if (j.find("sigma") != j.end()) {
      sigma = j.at("sigma").get<arma::Mat<double>>();
      if (sigma.n_elem != n_dims * n_dims) {
        std::domain_error("`sigma` doesn't have the correct size.");
      }
    } else {
      if (j.find("stddevs") == j.end() || j.find("covs") == j.end())
        std::invalid_argument(
            "Either `sigma` or `covs` and `stddevs` have to be given.");

      arma::rowvec stddevs = get_expr_setup_params(j.at("stddevs"), n_dims);
      arma::rowvec covs =
      get_expr_setup_params(j.at("covs"), n_dims * (n_dims - 1) / 2);
      sigma = constructCovMatrix(stddevs, covs, n_dims);
    }

    if (j.find("lowers") == j.end() || j.find("uppers") == j.end())
      std::invalid_argument("lower or upper boundaries are missing.");

    lowers = get_expr_setup_params(j.at("lowers"), n_dims);
    uppers = get_expr_setup_params(j.at("uppers"), n_dims);

    return baaraan::truncated_mvnorm_distribution<T>{means.t(), sigma, lowers,
                                                     uppers};
  }
};

} // namespace nlohmann

#endif // SAMPP_UTILITIES_H
