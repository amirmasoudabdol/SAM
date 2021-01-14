//===-- Parameter.cpp - Parameter Implementation --------------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 14-07-2020
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the implementation of Parameter template class.
///
//===----------------------------------------------------------------------===//

#include "Parameter.h"

using namespace sam;


/// This constructs **and** initializes the Parameter object based on the given 
/// JSON object. 
///
/// @param[in]  j     The configuration of the parameter
/// @param[in]  size  The size of the array
///
/// @tparam     T     The type of the Parameter, a.k.a, the type of armadillo
/// vector
template <typename T>
Parameter<T>::Parameter(const json &j, size_t size) {
  
  this->resize(size);
  
  arma::Col<T> val(size);
  
  switch (j.type()) {
    case nlohmann::detail::value_t::array: {
      val = arma::Col<T>(j.get<std::vector<T>>());
    } break;
      
    case nlohmann::detail::value_t::number_integer:
    case nlohmann::detail::value_t::number_unsigned:
    case nlohmann::detail::value_t::number_float:
      val = std::vector<T>(size, j.get<T>());
      break;
      
    case nlohmann::detail::value_t::object: {
      
      if (!j.contains("dist"))
        throw std::invalid_argument("Please provide a distribution specification.\n");
      
      auto name = j.at("dist").get<std::string>();
      if (name.find("mv") != std::string::npos) {
        // Multivariate Distribution
        dist = make_multivariate_distribution(j);
        auto v = Random::get(std::get<2>(dist));
        val.imbue([&, i = 0]() mutable {
          return static_cast<T>(v[i++]);
        });
      } else if (name.find("distribution") != std::string::npos) {
        // Univariate Distribution
        dist = make_distribution(j);
        auto v = static_cast<T>(Random::get(std::get<1>(dist)));
        val = arma::Col<T>(std::vector<T>(size, v));
      }
    } break;
      
    case nlohmann::detail::value_t::null:
    default:
      throw std::invalid_argument("Missing parameter.\n");
  }
  
  // Fill in the array values into `this`
  this->imbue([&, i = 0]() mutable {
    return val[i++];
  });
}

/// In the case of Parameter being initialized by a distribution, this method 
/// will be able to randomize its values everytime that is being called.
///
/// @attention This will be called everytime any of the cast operators are 
/// called.
///
/// @tparam     T     Type of the Parameter
template <typename T>
void Parameter<T>::randomize() {
  if (dist.index() != 0) {
    std::visit(overload {
      [&](Distribution &d) {
        auto v = static_cast<T>(Random::get(d));
        this->fill(v);
      },
      [&](MultivariateDistribution &md) {
        auto v = Random::get(md);
        this->imbue([&, i = 0]() mutable {
          return static_cast<T>(v[i++]);
        });
      },
      [&](std::monostate &m) {
        // If there is no distribution, we don't randomize, and the values will
        // stay intact
      }
    }, dist);
  }
}

namespace sam {
  template class Parameter<int>;
  template class Parameter<double>;
}


/// @todo Implement a JSON serializer for Parameter<T>
