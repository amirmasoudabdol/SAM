//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENTSETUP_H
#define SAMPP_EXPERIMENTSETUP_H

#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <cmath>

#include <armadillo>

#include "TestStrategy.h"
#include "RandomNumberGenerator.h"

#include "nlohmann/json.hpp"

namespace sam {

    using json = nlohmann::json;

    /**
     * @brief      Specifying different type of expeirments.
     */
    enum class ExperimentType {
        LinearModel,    ///< Linear Model, with or without covarinace
        LatentModel     ///< Latent Model or Structural Equation Model
    };

    const std::map<std::string, ExperimentType>
    stringToExperimentType = {
        {"LinearModel", ExperimentType::LinearModel},
        {"LatentModel", ExperimentType::LatentModel}
    };


    /**
     @brief Define a class for ExperimentSetup.
     
     ExperimentSetup contains the necessary parameters for initiating and
     generating the data needed for the Experiment.
     */
    class ExperimentSetup {
        
        //! Main random number stream used by the class to randomize `true_nobs`, etc.
        RandomNumberGenerator *rng_stream;
        
        //! Number of experimental conditions, e.g., treatment 1, treatment 2.
        int nc_ = 1;
        
        //! Number of _dependent variables_ in each experimental condition.
        int nd_ = 3;
        
        //! Number of items for each latent variable, if `isFactorModel` is `true`.
        int ni_ = 0;
        
        //! Total number of groups. Always calculated as
        //! \f$n_g = n_c \times n_d\f$, unless the simulation contains latent
        //! variables, \f$n_g = n_c \times n_d \times n_i\f$
        int ng_;
        
        //! Total number of groups in the case of latent experiment.
        //! This is a helper variable and doesn't mean anything conceptually
        int nrows_;
        
        void updateExperimentSize() {
            ng_ = nc_ * nd_;
            nrows_ = ng_ * ni_;
            
            initializeMemory();
        }
        
        std::unordered_map<std::string, arma::Mat<double>> true_parameters_;
        
        void initializeMemory() {
            true_parameters_["nobs"] = arma::Row<double>(ng_, arma::fill::zeros);
            true_parameters_["means"] = arma::Row<double>(ng_, arma::fill::zeros);
            true_parameters_["vars"] = arma::Row<double>(ng_, arma::fill::zeros);
            true_parameters_["sigma"] = arma::Mat<double>(ng_, ng_, arma::fill::zeros);
            
            true_parameters_["loadings"] = arma::Row<double>(ni_, arma::fill::zeros);
            true_parameters_["error_means"] = arma::Row<double>(nrows_, arma::fill::zeros);
            true_parameters_["error_vars"] = arma::Row<double>(nrows_, arma::fill::zeros);
            true_parameters_["error_sigma"] = arma::Row<double>(nrows_, arma::fill::zeros);
            
            // These two are arbitraty and only being used to generalize the setup
            true_parameters_["covs"] = arma::Row<double>(ng_, arma::fill::zeros);
            true_parameters_["error_cov"] = arma::Mat<double>(nrows_, nrows_, arma::fill::zeros);
            
            rng_stream = new RandomNumberGenerator(rand());
        }
        
        
        void is_valid_parameter_name(const std::string &pname) {
            if (true_parameters_.count(pname) == 0)
                throw std::invalid_argument("Unknown parameter.");
        }
        
    public:
        
        ExperimentType experiment_type;
        TestMethod test_method;
        
        //! Indicates whether `nobs` is should be selected as random
        bool is_n_randomized = false;
        
        explicit ExperimentSetup() {
            
            updateExperimentSize();
            
            initializeMemory();
        };
        
        ExperimentSetup(json& config);
        
        ExperimentSetup(int nc, int nd, int ni = 0)
            : nc_(nc), nd_(nd), ni_(ni) {
                
            updateExperimentSize();
            
        };
        
        ExperimentSetup(int nc, int nd,
                        int nobs, double means, double vars, double covs = 0.0)
        : nc_(nc), nd_(nd) {
            
            updateExperimentSize();
            
            setValueOf("nobs", nobs);
            setValueOf("means", means);
            setValueOf("vars", vars);
            
            auto sigma = constructCovMatrix(vars, covs);
            setValueOf("sigma", sigma);

        };
        
        
        ExperimentSetup(int nc, int nd,
                        arma::Row<double> nobs, arma::Row<double> means,
                        arma::Row<double> vars, double covs = 0.0)
        : nc_(nc), nd_(nd) {
            
            updateExperimentSize();
            
            if (nobs.n_cols != ng() || means.n_cols != ng() || vars.n_cols != ng())
                throw std::length_error("Sizes do not match!");
            
            setValueOf("nobs", nobs);
            setValueOf("means", means);
            setValueOf("vars", vars);
            
            auto sigma = constructCovMatrix(vars, covs);
            setValueOf("sigma", sigma);
            
            
        }
        
        ExperimentSetup(int nc, int nd,
                        arma::Row<double> nobs, arma::Row<double> means,
                        arma::Mat<double> sigma)
        : nc_(nc), nd_(nd) {
            
            updateExperimentSize();
            
            if (nobs.n_cols != ng() || means.n_cols != ng()
                || sigma.n_rows != ng() || sigma.n_cols != ng())
                throw std::length_error("Sizes do not match!");
            
            arma::vec vars = sigma.diag();
            
            setValueOf("nobs", nobs);
            setValueOf("means", means);
            setValueOf("vars", vars);
            setValueOf("sigma", sigma);
        }
        
        

        
        ~ExperimentSetup() = default;
        
        
        void randomize_nObs();
        
        std::vector<double> intervals;
        std::vector<double> weights;
        
        
        const int nc() const { return nc_; };
        const int nd() const { return nd_; };
        const int ni() const { return ni_; };
        const int ng() const { return ng_; };
        const int nrows() const { return nrows_; };
        
        void setSeed(int s) {
            rng_stream->setSeed(s);
        }
        
        
        arma::Mat<double> constructCovMatrix(double var, double cov);
        arma::Mat<double> constructCovMatrix(arma::Row<double> vars, double cov);
        
        /**
         Set values of `pname` to a constant value

         @param pname The parameter name
         @param val   A constant value
         */
        void setValueOf(std::string pname, double val);
        
        
        /**
         Replace values of `pname` with the given matrix

         @param pname The parameter name
         @param val_v A matrix
         */
        void setValueOf(std::string pname, arma::Mat<double>& val_v);
        
        
        /**
         Set values of `pname` by drawing from a uniform_int(min, max)

         @param pname The parameter name
         @param min The lower bound of the uniform distribution
         @param max The upper bound of the uniform distirbution
         */
        void setValueOf(std::string pname, int min, int max);
        
        
        /**
         Set values of `pname` using the given function

         @param pname The parameter name
         @param std::function<double(void)> A function returning a double
         */
        void setValueOf(std::string pname, std::function<double(void)> fun);
        
        
        /**
         Return values of the selected parameter

         @param pname The name of the parameter
         @return A const reference to the value
         */
        const arma::Mat<double>& getValueOf(std::string pname) {
            is_valid_parameter_name(pname);
            
            return true_parameters_[pname];
        }
        
    };

}

#endif //SAMPP_EXPERIMENTSETUP_H
