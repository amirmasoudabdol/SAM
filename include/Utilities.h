//
// Created by Amir Masoud Abdol on 2019-05-29.
//

#ifndef SAMPP_UTILITIES_H
#define SAMPP_UTILITIES_H

#include <vector>
#include <memory>
#include <random>
#include <iostream>
#include <algorithm>

#include "nlohmann/json.hpp"
#include "utils/magic_enum.hpp"

using json = nlohmann::json;
using Generator = std::mt19937;

class DistributionRandomGenerator {
public:
    DistributionRandomGenerator();
    
    
};

template<typename T>
std::vector<T> get_expr_setup_params(json const &j, int const size);

template<typename T>
T get_enum_value_from_json(const std::string &key, const json &j) {
    auto name = magic_enum::enum_cast<T>(j[key].get<std::string>());
    if (name.has_value())
        return name.value();
    else
        throw std::invalid_argument("Unknown value.");
}

#endif //SAMPP_UTILITIES_H
