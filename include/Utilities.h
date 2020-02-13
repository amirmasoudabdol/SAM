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
#include "effolkronium/random.hpp"
#include "mvrandom.hpp"

using json = nlohmann::json;
using Generator = std::mt19937;
using Distribution = std::function<double(Generator &)>;
using MultivariateDistribution = std::function<arma::mat(Generator &)>;
using Random = effolkronium::random_static;

Distribution make_distribution(json const &j);
MultivariateDistribution make_multivariate_distribution(json const &j);

template<typename T = double>
std::vector<T>
get_expr_setup_params(json const &j, int const size) {
    
    switch (j.type()) {
        case nlohmann::detail::value_t::array:
            {
                if (j.size() != size)
                    throw std::length_error("Array size does not match with the size\
                                        of the experiment.\n");
                else
                    return j.get<std::vector<T>>();
            }
            break;
            
        case nlohmann::detail::value_t::number_integer:
        case nlohmann::detail::value_t::number_unsigned:
        case nlohmann::detail::value_t::number_float:
            return std::vector<T>(size, j.get<T>());
            break;
            
        case nlohmann::detail::value_t::null:
        default:
            throw std::invalid_argument("Missing parameter.\n");
            break;
    }
    
}

template <class DistributionType, class... Parameters>
Distribution make_distribution_impl(json const &j, Parameters... parameters) {
    return DistributionType{j.at(parameters)...};
}

template <class DistributionType, class... Parameters>
MultivariateDistribution make_multivariate_distribution_impl(json const &j, Parameters... parameters) {
    return DistributionType{j.at(parameters)...};
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

arma::Mat<double>
constructCovMatrix(const arma::Row<double> &stddevs, const arma::Row<double> &covs, int n);

arma::Mat<double>
constructCovMatrix(const arma::Row<double> &stddevs, const double cov, int n);

arma::Mat<double>
constructCovMatrix(const double stddev, const double cov, const int n);

namespace arma {
    
    /// Set of serializer for converting arma::row, arma::rowvec, and arma::mat
    /// to JSON objects.

    template <typename T>
    void to_json(json &j, const arma::Row<T> &row) {
        j = arma::conv_to<std::vector<T>>::from(row);
    }

    template<typename T>
    void from_json(const json& j, arma::Row<T> &row) {
        row = arma::Row<T>(j.get<std::vector<T>>());
    }

    template <typename T>
    void to_json(json &j, const arma::Col<T> &col) {
        for (int i{0}; i < col.n_rows; i++){
            j.push_back(arma::conv_to<std::vector<T>>::from(col.row(i)));
        }
    }

    template<typename T>
    void from_json(const json& j, arma::Col<T> &col) {
        col = arma::Col<T>(j.size());
        for (int i{0}; i < j.size(); i++) {
            assert(j[i].size() == 1);
            col.at(i) = j[i].get<std::vector<T>>()[0];
        }
    }

    template <typename T>
    void to_json(json &j, const arma::Mat<T> &mat) {
        
        for (int i{0}; i < mat.n_rows; i++){
            j.push_back(arma::conv_to<std::vector<T>>::from(mat.row(i)));
        }

    }

    template <typename T>
    void from_json(const json &j, arma::Mat<T> &mat) {
        
        int n_rows = j.size();
        int n_cols = j[0].size();
        for (int i{0}; i < n_rows; i++)
            assert(j[i].size() == n_cols);
        
        mat = arma::Mat<T>(n_rows, n_cols);
        for (int i{0}; i < n_rows; i++){
            mat.row(i) = j[i].get<arma::Row<T>>();
        }

    }

}

namespace nlohmann {


    template <typename T>
    struct adl_serializer<mvrandom::mvnorm_distribution<T>> {
        
        static void to_json(json &j, const mvrandom::mvnorm_distribution<T> &mdist) {

            j["dist"] = "mvnorm_distribution";
            j["means"] = arma::rowvec(mdist.means().as_row());
            j["sigma"] = mdist.sigma();
            
        }

        static mvrandom::mvnorm_distribution<T> from_json(const json &j) {
            
            auto means = j.at("means").get<arma::Row<double>>();
            arma::Mat<T> sigma;
            auto n_dims = means.n_elem;
            
            if (j.find("sigma") != j.end()){
                sigma = j.at("sigma").get<arma::Mat<double>>();
                
            }else{
                if (j.find("stddevs") == j.end() || j.find("covs") == j.end()){
                    std::invalid_argument("Either `sigma` or `covs` and `stddevs` have to be given.");
                }
                arma::rowvec stddevs = get_expr_setup_params(j.at("stddevs"), n_dims);
                arma::rowvec covs = get_expr_setup_params(j.at("covs"), n_dims * (n_dims - 1) / 2);
                // construct sigma
                sigma = constructCovMatrix(stddevs, covs, n_dims);
            }

            return mvrandom::mvnorm_distribution<T>{means.t(), sigma};
        }

    };


    template <typename T>
    struct adl_serializer<mvrandom::truncated_mvnorm_distribution<T>> {
        
        static void to_json(json &j, const mvrandom::truncated_mvnorm_distribution<T> &mdist) {

            j["dist"] = "truncated_mvnorm_distribution";
            j["means"] = arma::rowvec(mdist.means().as_row());
            j["sigma"] = mdist.sigma();
            j["lowers"] = arma::rowvec(mdist.lowers().as_row());
            j["uppers"] = arma::rowvec(mdist.uppers().as_row());
            
        }

        static mvrandom::truncated_mvnorm_distribution<T> from_json(const json &j) {
            
            auto means = j.at("means").get<arma::Row<double>>();
            auto n_dims = means.n_elem;
            
            arma::Mat<T> sigma;
            arma::Mat<T> lowers, uppers;
            
            if (j.find("sigma") != j.end()){
                sigma = j.at("sigma").get<arma::Mat<double>>();
                if (sigma.n_elem != n_dims * n_dims) {
                    std::domain_error("`sigma` doesn't have the correct size.");
                }
            }else{
                if (j.find("stddevs") == j.end() || j.find("covs") == j.end())
                    std::invalid_argument("Either `sigma` or `covs` and `stddevs` have to be given.");
                
                arma::rowvec stddevs = get_expr_setup_params(j.at("stddevs"), n_dims);
                arma::rowvec covs = get_expr_setup_params(j.at("covs"), n_dims * (n_dims - 1) / 2);
                // construct sigma
                sigma = constructCovMatrix(stddevs, covs, n_dims);
            }
            
            if(j.find("lowers") == j.end() || j.find("uppers") == j.end())
                std::invalid_argument("lower or upper boundries are missing.");
            
            lowers = get_expr_setup_params(j.at("lowers"), n_dims);
            uppers = get_expr_setup_params(j.at("uppers"), n_dims);

            return mvrandom::truncated_mvnorm_distribution<T>{means.t(), sigma, lowers, uppers};

        }

    };

}





#endif //SAMPP_UTILITIES_H
