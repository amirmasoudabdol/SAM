//===-- Parameter.h - Random Parameter Implementation ---------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 14-07-2020
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This files contains the declaration of Parameter, which is an abstract
/// representation of random variable/parameter.
///
//===----------------------------------------------------------------------===//

#ifndef SAMPP_PARAMETER_H
#define SAMPP_PARAMETER_H

#include "sam.h"
#include "Utilities.h"
#include <variant>

namespace sam {

/// @brief  An abstract representation of a random variable parameter.
///
/// This is designed to account for those setting parameters that
/// can be configured in either of the following ways:
///
/// - An array of size _n_, with similar values
/// - An array of size _n_, with different values
/// - An array of size _n_, with values drawn from a distribution. This is the
///   case where Parameter can loosely resemble a random variable.
///   
/// @attention Parameter<T> is inherited from arma::Row<T>. This is quite useful and
/// essential because I can pass it around easily, and use all the overloaded 
/// operators of the arma::Row<T>
///
/// @tparam     T     Type of the parameter
template <typename T>
class Parameter : public arma::Row<T> {
  
  //! A variant storing either an instance of Distribution or
  //! MultivariateDistribution
  std::variant<std::monostate, Distribution, MultivariateDistribution> dist;
    
public:
  
  Parameter() : arma::Row<T>() {};

  Parameter(std::initializer_list<T> l) : arma::Row<T>(l) {};
  
  /// Constructs a parameter based on the given JSON object
  Parameter(const json &j, size_t size);
  
  /// @todo Implement a copy constructor

  /// Returns the _first_ element of the array
  explicit operator T() {
    this->randomize();
    return this->at(0);
  };
  
  /// Randomizes and returns the entire array of data
  explicit operator arma::Row<T>() {
    this->randomize();
    return this->head(this->n_elem);
  }
  
  /// Randomize the values of the arma::Row<T> container
  void randomize();
  
  /// Returns true if a distribution is assigned to the Parameter
  [[nodiscard]] bool isDist() {
    return not std::holds_alternative<std::monostate>(dist);
  };
  
};

}

#endif // SAMPP_PARAMETER_H
