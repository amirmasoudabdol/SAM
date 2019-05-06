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
        
        //! Total number of groups. Always calculated as
        //! \f$n_g = n_c \times n_d\f$, unless the simulation contains latent
        //! variables, \f$n_g = n_c \times n_d \times n_i\f$
        int ng_;
        
        //! Total number of groups in the case of latent experiment.
        //! This is a helper variable and doesn't mean anything conceptually
        int nrows_;
        
        // Experiment Parameters
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
        
        //! Test Strategy Parameters
        TestStrategyParameters test_strategy_parameters_;
        
        //! Data Strategy Parameters
        DataStrategyParameters data_strategy_parameters_;
        
        // TODO: We are not properly initialized, fix us!
        ExperimentType experiment_type = ExperimentType::LinearModel;
        TestType test_method = TestType::TTest;


        
        //! Indicates whether `nobs` should be selected as random
        bool is_n_randomized = false;
        

        //! This is just a handy default constructor that I should remove at
        //! some point!
        explicit ExperimentSetup() {
            updateExperimentSize();
        };
        
        ExperimentSetup(json& config);
        
        ExperimentSetup(const int nc, const int nd, const int ni = 0);
        
        ExperimentSetup(const int nc, const int nd,
                        const int nobs, const double means, const double vars, const double covs,
                        const TestStrategyParameters test_params, DataStrategyParameters data_params);
        
        
        ExperimentSetup(const int nc, const int nd,
                        const arma::Row<int> nobs, const arma::Row<double> means,
                        const arma::Row<double> vars, const double covs,
                        const TestStrategyParameters test_params, DataStrategyParameters data_params);
        
        
        ExperimentSetup(const int nc, const int nd,
                        const arma::Row<int> nobs, const arma::Row<double> means,
                        const arma::Mat<double> sigma,
                        const TestStrategyParameters test_params, const DataStrategyParameters data_params);
        
        
        ~ExperimentSetup() = default;
        
        
        void randomize_nObs();
        
        std::vector<double> intervals;
        std::vector<double> weights;
        
        
        const int nc() const { return nc_; };
        const int nd() const { return nd_; };
        const int ni() const { return ni_; };
        const int ng() const { return ng_; };
        const int nrows() const { return nrows_; };
        
        const arma::Row<int>& nobs() const { return nobs_; };
        void set_nobs(arma::Mat<int>& val) {nobs_ = val; };
        const arma::Row<double>& means() const { return means_; };
        void set_means(arma::Mat<double>& val) {means_ = val; };
        const arma::Row<double>& vars() const { return vars_; };
        void set_vars(arma::Mat<double>& val) {vars_ = val; };
        const arma::Mat<double>& sigma() const { return sigma_; };
        void set_sigma(arma::Mat<double>& val) {sigma_ = val; };
        
        const arma::Row<double>& loadings() const { return loadings_; };
        void set_loadings(arma::Mat<double>& val) {loadings_ = val; };
        const arma::Row<double>& error_means() const { return error_means_; };
        void set_error_means(arma::Mat<double>& val) {error_means_ = val; };
        const arma::Row<double>& error_vars() const { return error_vars_; };
        void set_error_vars(arma::Mat<double>& val) {error_vars_ = val; };
        const arma::Mat<double>& error_sigma() const { return error_sigma_; };
        void set_error_sigma(arma::Mat<double>& val) {error_sigma_ = val; };
        
        void setSeed(int s) {
            rng_stream->setSeed(s);
        }
        
        
        arma::Mat<double> constructCovMatrix(double var, double cov);
        arma::Mat<double> constructCovMatrix(arma::Row<double> vars, double cov);
        
    };

}

#endif //SAMPP_EXPERIMENTSETUP_H
