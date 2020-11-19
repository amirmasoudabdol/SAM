//
// Created by Amir Masoud Abdol on 14-07-2020
//

#ifndef SAMPP_PARAMETER_H
#define SAMPP_PARAMETER_H

#include "sam.h"
#include "Utilities.h"
#include <variant>

namespace sam {

/// \brief An abstract class for a random variable parameter.
///
/// This is designed to capture the encapsulate a distribution and therefore
/// mimic the behavior or a random variable.
///
/// \todo Implement a copy constructor that can handle the copy
/// from arma::Row<T>
/// \todo Implement a double operator()()
template <typename T>
class Parameter : public arma::Row<T> {
  
  std::variant<std::monostate, Distribution, MultivariateDistribution> dist;
  
public:
  
  Parameter() : arma::Row<T>() {};
  
  Parameter(std::initializer_list<T> l) : arma::Row<T>(l) {};
  
  Parameter(const json &j, size_t size);
  
  void randomize();
  
  operator T() {
    this->randomize();
    return this->at(0);
  };
  
  bool isDist() {
    return not std::holds_alternative<std::monostate>(dist);
  };
  
};

}

#endif // SAMPP_PARAMETER_H
