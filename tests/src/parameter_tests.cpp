//===-- parameter_tests - Parameter Unit Tests ---------------------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 14/01/2021
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains some tests for Parameter<T>
///
//===----------------------------------------------------------------------===//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE Parameter Tests

#include "nlohmann/json.hpp"
#include "Parameter.h"
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <boost/utility/identity_type.hpp>

using namespace sam;
using json = nlohmann::ordered_json;

typedef boost::mpl::list<int, float, double> test_types;
typedef boost::mpl::list<float, double> dist_test_types;


BOOST_AUTO_TEST_CASE_TEMPLATE( parameter_type_test, T, test_types) {

  Parameter<T> parameter{1};

  BOOST_TEST(parameter.n_elem == 1);
  BOOST_TEST(sizeof(parameter.at(0)) == sizeof(T));
}

BOOST_AUTO_TEST_CASE_TEMPLATE( scalar_parameter, T, test_types) {

  Parameter<T> parameter{1};

  T b = static_cast<T>(parameter);

  // If the Parameter is not a distirbution, then, it'll not be randomized
  BOOST_TEST(static_cast<T>(parameter()) == b);

  json config = R"(
    {
      "param": 1
    }
  )"_json;
  
  Parameter<T> jparameter(config["param"], 1);
  BOOST_TEST(jparameter.at(0) == 1);

  jparameter = {2};
  BOOST_TEST(jparameter.at(0) == 2);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( array_of_scalars_parameter, T, test_types) {

  Parameter<T> parameter{1, 2, 3};

  arma::Row<T> b = parameter;

  // If the Parameter is not a distirbution, then, it'll not be randomized
  BOOST_TEST(arma::approx_equal(parameter, b, "absdiff", 0.001));

  json config = R"(
    {
    "param": [1, 2, 3]
    }
  )"_json;

  Parameter<T> jparameter(config["param"], 3);

  arma::Row<T> a{1, 2, 3};
  BOOST_TEST(arma::approx_equal(jparameter,
                                a, "absdiff", 0.001));
  
  a = arma::Row<T>({4, 5, 6});
  BOOST_TEST(!arma::approx_equal(jparameter,
                                a, "absdiff", 0.001));

}

BOOST_AUTO_TEST_CASE_TEMPLATE( univariate_parameter, T, dist_test_types) {

  json config = R"(
    {
      "param": {
        "dist": "normal_distribution",
        "mean": 0,
        "stddev": 1
      }
    }
  )"_json;

  Parameter<T> parameter(config["param"], 1);

  BOOST_TEST(parameter.n_elem == 1);

  T a = static_cast<T>(parameter());
  T b = static_cast<T>(parameter());
  BOOST_TEST(sizeof(a) == sizeof(b));
  BOOST_TEST(a != b);
}


BOOST_AUTO_TEST_CASE_TEMPLATE( multivariate_parameter, T, dist_test_types) {

  json config = R"(
  {
    "param": {
      "dist": "mvnorm_distribution",
      "means": [0, 0, 0, 0],
      "covs": 0,
      "stddevs": 1
    }
  }
  )"_json;

  Parameter<T> parameter(config["param"], 4);

  BOOST_TEST(parameter.n_elem == 4);

  arma::Row<T> a = parameter();
  arma::Row<T> b = parameter();

  BOOST_TEST(!arma::approx_equal(a, b, "absdiff", 0.001));
}

