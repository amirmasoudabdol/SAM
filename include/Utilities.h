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
//#include "utils/magic_enum.hpp"
#include "effolkronium/random.hpp"

using json = nlohmann::json;
using Generator = std::mt19937;
using Distribution = std::function<double(Generator &)>;
using MultivariateDistribution = std::function<arma::mat(Generator &)>;
using Random = effolkronium::random_static;


// JSON to ARMA Serializer
namespace nlohmann {
    template <typename T>
    struct adl_serializer<arma::Mat<T>> {
        static void to_json(json& j, const arma::Mat<T> &mat) {
            std::vector<std::vector<T>> vmat(mat.n_rows);
            
            for (int i = 0; i < mat.n_rows; ++i)
                vmat[i] = arma::conv_to<std::vector<T>>::from(mat.row(i));

            j = *mat;

        }
        
        static void from_json(const json& j, arma::Mat<T>& mat) {
            /// TODO: Check it vac is actually a matrix.
            /// TODO: This is almost working but I need to translate
            /// everything into `arma::vec` to make this more uniform all the way.
            int n_rows, n_cols;
            std::vector<std::vector<T>> vmat;
            if (j[0].type() != detail::value_t::array) {
                vmat.push_back(j.get<std::vector<T>>());
            }else if (j[0].type() == detail::value_t::array) {
                vmat = j.get<std::vector<std::vector<T>>>();
            }else{
                throw std::invalid_argument("Cannot convert the given value to a matrix."); // It's not a matrix
            }
            
            n_rows = vmat.size();
            n_cols = vmat[0].size();
            
            /// TODO: This is dirty but it return the transpose of the matrix, so
            /// it'll be `arma::vec` instead of `vec`.
            /// FIXME!
            mat = arma::Mat<T>(n_cols, n_rows);
            for (int i = 0; i < n_rows; ++i) {
                for (int j = 0; j < n_cols; ++j) {
                    mat(j, i) = vmat[i][j];
                }
            }
            
        }
    };
}

Distribution make_distribution(json const &j);
MultivariateDistribution make_multivariate_distribution(json const &j);

template <class DistributionType, class... Parameters>
Distribution make_distribution_impl(json const &j, Parameters... parameters) {
    return DistributionType{j.at(parameters)...};
}

template <class DistributionType, class... Parameters>
MultivariateDistribution make_multivariate_distribution_impl(json const &j, Parameters... parameters) {
    return DistributionType{j.at(parameters)...};
}

template<typename T = double>
std::tuple<std::vector<T>, Distribution>
get_expr_setup_params(json const &j, int const size) {
    
    switch (j.type()) {
        case nlohmann::detail::value_t::object:
            try {
                
                auto dist = make_distribution(j);
                auto val = Random::get(dist);
                
                return std::make_tuple(std::vector<T>(size, val), dist);
                
            } catch (const std::exception& e) {
                throw e.what();
            }
            break;
            
        case nlohmann::detail::value_t::array:
            {
                if (j.size() != size)
                    throw std::length_error("Array size does not match with the size\
                                        of the experiment.\n");
                else
                    return std::make_tuple(j.get<std::vector<T>>(), nullptr);
            }
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
 
 TODO: Just use the return value here, passing by reference is not safe and
 necessary, also due to the move sematic, I can just do this easily without
 preformance lose.
 */
template<typename T>
void fill_vector(arma::Row<T> &vecc, int size, T val){
    vecc = arma::Row<T>(size).fill(val);
};


//template<typename T>
//T get_enum_value_from_json(const std::string &key, const json &j) {
//    auto name = magic_enum::enum_cast<T>(j[key].get<std::string>());
//    if (name.has_value())
//        return name.value();
//    else
//        throw std::invalid_argument("Unknown value.");
//}

arma::Mat<double>
constructCovMatrix(const arma::Row<double> &vars, const arma::Row<double> &covs, int n);

arma::Mat<double>
constructCovMatrix(const arma::Row<double> &vars, const double cov, int n);

arma::Mat<double>
constructCovMatrix(const double var, const double cov, const int n);

#endif //SAMPP_UTILITIES_H
