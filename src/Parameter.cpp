//
// Created by Amir Masoud Abdol on 14-07-2020
//

#include "Parameter.h"

using namespace sam;

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
      std::string name = j.at("dist").get<std::string>();
      if (name.find("mv") != std::string::npos) {
        /// Multivariante Distribution
        dist = make_multivariate_distribution(j);
        auto v = Random::get(std::get<2>(dist));
        val.imbue([&, i = 0]() mutable {
          return static_cast<T>(v[i++]);
        });
      }else{
        /// Univariate Distribution
        dist = make_distribution(j);
        auto v = static_cast<T>(Random::get(std::get<1>(dist)));
        val = arma::Col<T>(std::vector<T>(size, v));
      }
    } break;
      
    case nlohmann::detail::value_t::null:
    default:
      throw std::invalid_argument("Missing parameter.\n");
      break;
  }
  
  this->imbue([&, i = 0]() mutable {
    return val[i++];
  });
}

template <typename T>
void Parameter<T>::randomize() {
  if (dist.index() != 0) {
    std::visit(overload {
      [&](Distribution &d) {
        auto v = static_cast<T>(Random::get(d));
        this->fill(v);
        return;
      },
      [&](MultivariateDistribution &md) {
        auto v = Random::get(md);
        this->imbue([&, i = 0]() mutable {
          return static_cast<T>(v[i++]);
        });
        return;
      },
      [&](auto &monostate) {
        return;
      }
    }, dist);
  }
}

namespace sam {
  template class Parameter<int>;
//  template class Parameter<double>;
}
