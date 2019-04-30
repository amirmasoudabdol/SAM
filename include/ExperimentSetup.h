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
        void updateNumGroups() {
            ng_ = nc_ * nd_;
        }
        
        //! Total number of groups in the case of latent experiment.
        //! This is a helper variable and doesn't mean anything conceptually
        int nrows_;
        void updateNumRows() {
            nrows_ = ng_ * ni_;
        }
        
        std::unordered_map<std::string, arma::Mat<double>> true_parameters_;
        
        void initializeMemory() {
            true_parameters_["nobs"] = arma::Row<double>(ng_);
            true_parameters_["means"] = arma::Row<double>(ng_);
            true_parameters_["vars"] = arma::Row<double>(ng_);
            true_parameters_["covs"] = arma::Row<double>(ng_);
            
            true_parameters_["sigma"] = arma::Mat<double>(ng_, ng_);
            
            true_parameters_["loadings"] = arma::Row<double>(ni_);
            true_parameters_["error_means"] = arma::Row<double>(nrows_);
            true_parameters_["error_vars"] = arma::Row<double>(nrows_);
            true_parameters_["error_cov"] = arma::Mat<double>(nrows_, nrows_);
        }
        
    public:
        
        ExperimentType experiment_type;
        TestMethod test_method;
        
        //! Indicates whether `nobs` is should be selected as random
        bool is_n_randomized = false;
        
        ExperimentSetup() = default;
        
        ExperimentSetup(json& config);
        
        ExperimentSetup(int nc, int nd, int ni = 0)
            : nc_(nc), nd_(nd), ni_(ni) {
                
                updateNumGroups();
                updateNumRows();
            
                initializeMemory();
                
                
            };
        

        
        ~ExperimentSetup() = default;
        
        
        void randomize_nObs();
        
        std::vector<double> intervals;
        std::vector<double> weights;
        
        
        int nc() const { return nc_; };
        int nd() const { return nd_; };
        int ni() const { return ni_; };
        int ng() const { return ng_; };
        int nrows() const { return nrows_; };
        
        void setExperimentSize(int nc, int nd, int ni = 0) {
            
            nc_ = nc;
            nd_ = nd;
            updateNumGroups();
            
            // If ExperimentType == LatentModel
            ni_ = ni;
            updateNumRows();
            
            initializeMemory();
        }
        

        
        
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
            return true_parameters_[pname];
        }
        
    };

}

#endif //SAMPP_EXPERIMENTSETUP_H
