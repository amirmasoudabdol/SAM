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
      auto name = j.at("dist").get<std::string>();
      if (name.find("mv") != std::string::npos) {
        /// Multivariate Distribution
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
      },
      [&](MultivariateDistribution &md) {
        auto v = Random::get(md);
        this->imbue([&, i = 0]() mutable {
          return static_cast<T>(v[i++]);
        });
      },
      [&](std::monostate &m) { }
    }, dist);
  }
}

namespace sam {
  template class Parameter<int>;
  template class Parameter<double>;
}


/// I'm not so sure if this is online, in a sense that it's being used
/// by nlohmann::json
namespace nlohmann {

template <typename T>
struct adl_serializer<sam::Parameter<T>> {
  
  static void to_json(json &j, const sam::Parameter<T> &p) {
    j = static_cast<T>(p);
  }
  
  static void from_json(const json &j, sam::Parameter<T> &p) {
    p = Parameter<T>(j, 1);
  }
};

}
