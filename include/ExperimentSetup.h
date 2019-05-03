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

#include "DataStrategy.h"
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
    
    struct DataStrategyParameters {
        ExperimentType name;
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
        
        TestStrategyParameters test_strategy_parameters_;
        DataStrategyParameters data_strategy_parameters_;
        
        //! Total number of groups. Always calculated as
        //! \f$n_g = n_c \times n_d\f$, unless the simulation contains latent
        //! variables, \f$n_g = n_c \times n_d \times n_i\f$
        int ng_;
        
        //! Total number of groups in the case of latent experiment.
        //! This is a helper variable and doesn't mean anything conceptually
        int nrows_;
        
        
        arma::Row<int> nobs_;
        arma::Row<double> means_;
        arma::Row<double> vars_;
        arma::Mat<double> sigma_;
        
        arma::Row<double> loadings_;
        arma::Row<double> error_means_;
        arma::Row<double> error_vars_;
        arma::Mat<double> error_sigma_;
        
        void updateExperimentSize() {
            ng_ = nc_ * nd_;
            nrows_ = ng_ * ni_;
        }
        
    public:
        
        // TODO: We are not properly initialized, fix us!
        ExperimentType experiment_type = ExperimentType::LinearModel;
        TestType test_method = TestType::TTest;


        
        //! Indicates whether `nobs` is should be selected as random
        bool is_n_randomized = false;
        
        explicit ExperimentSetup() {
            
            updateExperimentSize();
        };
        
        ExperimentSetup(json& config);
        
        ExperimentSetup(int nc, int nd, int ni = 0)
            : nc_(nc), nd_(nd), ni_(ni)
        {
                
            updateExperimentSize();
            
            test_strategy_parameters_.name = TestType::TTest;
            data_strategy_parameters_.name = ExperimentType::LinearModel;
        };
        
        ExperimentSetup(int nc, int nd,
                        int nobs, double means, double vars, double covs,
                        TestStrategyParameters test_params, DataStrategyParameters data_params)
        : nc_(nc), nd_(nd), ni_(0), test_strategy_parameters_(test_params), data_strategy_parameters_(data_params)
        {
            updateExperimentSize();
            
            nobs_ = arma::Row<int>(ng_).fill(nobs);
            means_ = arma::Row<double>(ng_).fill(means);
            vars_ = arma::Row<double>(ng_).fill(vars);
            
            auto sigma = constructCovMatrix(vars, covs);
            sigma_ = sigma;
            
        };
        
        
        ExperimentSetup(int nc, int nd,
                        arma::Row<int> nobs, arma::Row<double> means,
                        arma::Row<double> vars, double covs,
                        TestStrategyParameters test_params, DataStrategyParameters data_params)
        : nc_(nc), nd_(nd), ni_(0), test_strategy_parameters_(test_params), data_strategy_parameters_(data_params)
        {
            updateExperimentSize();
            
            if (nobs.n_cols != ng() || means.n_cols != ng() || vars.n_cols != ng())
                throw std::length_error("Sizes do not match!");
            
            nobs_ = nobs;
            means_ = means;
            vars_ = vars;
            
            auto sigma = constructCovMatrix(vars, covs);
            sigma_ = sigma;
            
            
        }
        
        ExperimentSetup(int nc, int nd,
                        arma::Row<int> nobs, arma::Row<double> means,
                        arma::Mat<double> sigma,
                        TestStrategyParameters test_params, DataStrategyParameters data_params)
        : nc_(nc), nd_(nd), ni_(0), test_strategy_parameters_(test_params), data_strategy_parameters_(data_params)
        {
            updateExperimentSize();
            
            if (nobs.n_cols != ng() || means.n_cols != ng()
                || sigma.n_rows != ng() || sigma.n_cols != ng())
                throw std::length_error("Sizes do not match!");
                        
            nobs_ = nobs;
            means_ = means;
            vars_ = sigma.diag().t();
            sigma_ = sigma;
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
        
        const arma::Row<int>& nobs() { return nobs_; };
        void set_nobs(arma::Mat<int>& val) {nobs_ = val; };
        const arma::Row<double>& means() { return means_; };
        void set_means(arma::Mat<double>& val) {means_ = val; };
        const arma::Row<double>& vars() { return vars_; };
        void set_vars(arma::Mat<double>& val) {vars_ = val; };
        const arma::Mat<double>& sigma() { return sigma_; };
        void set_sigma(arma::Mat<double>& val) {sigma_ = val; };
        
        const arma::Row<double>& loadings() { return loadings_; };
        void set_loadings(arma::Mat<double>& val) {loadings_ = val; };
        const arma::Row<double>& error_means() { return error_means_; };
        void set_error_means(arma::Mat<double>& val) {error_means_ = val; };
        const arma::Row<double>& error_vars() { return error_vars_; };
        void set_error_vars(arma::Mat<double>& val) {error_vars_ = val; };
        const arma::Mat<double>& error_sigma() { return error_sigma_; };
        void set_error_sigma(arma::Mat<double>& val) {error_sigma_ = val; };
        
        void setSeed(int s) {
            rng_stream->setSeed(s);
        }
        
        
        arma::Mat<double> constructCovMatrix(double var, double cov);
        arma::Mat<double> constructCovMatrix(arma::Row<double> vars, double cov);
        
    };

}

#endif //SAMPP_EXPERIMENTSETUP_H
