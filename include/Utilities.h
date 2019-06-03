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
#include <tuple>

#include "sam.h"

#include "nlohmann/json.hpp"
#include "utils/magic_enum.hpp"

using json = nlohmann::json;
using Generator = std::mt19937;
using Distribution = std::function<double(Generator &)>;

Distribution make_distribution(json const &j);


template <class DistributionType, class... Parameters>
Distribution make_distribution_impl(json const &j, Parameters... parameters) {
    return DistributionType{j.at(parameters)...};
}

template<typename T>
std::tuple<std::vector<T>, Distribution>
get_expr_setup_params(json const &j, int const size) {
    
    switch (j.type()) {
        case nlohmann::detail::value_t::object:
            try {
                Generator gen{std::random_device{}()};
                
                auto dist = make_distribution(j);
                auto val = dist(gen);
                
                return std::make_tuple(std::vector<T>(size, val), dist);
                
            } catch (const std::exception& e) {
                throw e.what();
            }
            break;
            
        case nlohmann::detail::value_t::array:
            if (j.size() != size)
                throw std::length_error("Array size does not match with the size\
                                        of the experiment.\n");
            else
                return std::make_tuple(j.get<std::vector<T>>(), nullptr);
            break;
            
        case nlohmann::detail::value_t::number_integer:
        case nlohmann::detail::value_t::number_unsigned:
        case nlohmann::detail::value_t::number_float:
            return std::make_tuple(std::vector<T>(size, j.get<T>()), nullptr);
            break;
            
        case nlohmann::detail::value_t::null:
        default:
            throw std::invalid_argument("Missing parameter.\n");
            break;
    }
    
}

/**
 \brief      A helper function to fill `val` to a vector. This
 will also allocate the necessary space, `size`, for the vector.
 
 \param      vecc  The reference to the vector
 \param[in]  size  The size of the vector
 \param[in]  val   The value of the vector
 
 \tparam     T     The type of the vector and value.
 
 TODO: This can move out!
 TODO: Just use the return value here, passing by reference is not safe and
 necessary, also due to the move sematic, I can just do this easily without
 preformance lose.
 */
template<typename T>
void fill_vector(arma::Row<T> &vecc, int size, T val){
    vecc = arma::Row<T>(size).fill(val);
};


template<typename T>
T get_enum_value_from_json(const std::string &key, const json &j) {
    auto name = magic_enum::enum_cast<T>(j[key].get<std::string>());
    if (name.has_value())
        return name.value();
    else
        throw std::invalid_argument("Unknown value.");
}

#endif //SAMPP_UTILITIES_H
