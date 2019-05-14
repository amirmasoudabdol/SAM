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
/**/
    using json = nlohmann::json;
    
    class DataStrategy;
    class TestStrategy;

    class ExperimentSetupBuilder;

    /**
     @brief Define a class for ExperimentSetup.

     ExperimentSetup contains the necessary parameters for initiating and
     generating the data needed for the Experiment.
     */
    class ExperimentSetup {

        friend class ExperimentSetupBuilder;

        //! Main random number stream used by the class to randomize `true_nobs`, etc.
        RandomNumberGenerator *rng_stream;

        //! Number of experimental conditions, e.g., treatment 1, treatment 2.
        int nc_ = 0;

        //! Number of _dependent variables_ in each experimental condition.
        int nd_ = 0;

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

        
        /**
         Create a new ExperimentSetup by invoking a ExperimentSetupBuilder.

         @return An instance of the builder.
         */
        static ExperimentSetupBuilder create();

        //! Test Strategy Parameters
        TestStrategy::TestStrategyParameters tsp_;

        //! Data Strategy Parameters
        DataStrategy::DataStrategyParameters dsp_;

        //! Indicates whether `nobs` should be selected as random
        bool is_n_randomized = false;

        //! This is just a handy default constructor that I should remove at
        //! some point!
        ExperimentSetup() = default;

        explicit ExperimentSetup(json& config);

        ~ExperimentSetup() = default;


        void randomize_nObs();

        std::vector<double> intervals;
        std::vector<double> weights;
        
        void setSeed(int s) {
            rng_stream->setSeed(s);
        }
        
        arma::Mat<double> constructCovMatrix(double var, double cov) const;
        arma::Mat<double> constructCovMatrix(const arma::Row<double> &vars, double cov) const;


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

    };


    class ExperimentSetupBuilder {
        
        ExperimentSetup setup;

        bool is_expr_size_decided = false;
        bool is_vars_set = false;

        void calculate_experiment_size() {
            setup.ng_ = setup.nc_ * setup.nd_;
            setup.nrows_ = setup.ng_ * setup.ni_;

            if (setup.ng_ == 0){
                is_expr_size_decided = false;
            }else{
                is_expr_size_decided = true;
            }
        }

        void check_expr_size() {
            if (!is_expr_size_decided) {
                calculate_experiment_size();
                if (!is_expr_size_decided){
                    throw std::invalid_argument("Number of groups cannot be 0. Make sure that \
                            you have set the number of conditions or dependent variables first, with non-zero \
                            values.");
                }
            }
        }

        /**
         * \brief      A helper function to fill `val` to a vector. This
         * will also allocate the necessary space, `size`, for the vector.
         *
         * \param      vecc  The reference to the vector
         * \param[in]  size  The size of the vector
         * \param[in]  val   The value of the vector
         *
         * \tparam     T     The type of the vector and value.
         */
        template<typename T>
        void fill_vector(arma::Row<T> &vecc, int size, T val){
            vecc = arma::Row<T>(size).fill(val);
        };

    public:

        ExperimentSetupBuilder() = default;

        ExperimentSetupBuilder& fromConfigFile(json &config);

        ExperimentSetupBuilder& setNumConditions(const int nc) {
            if (nc <= 0) {
                throw std::invalid_argument("Number of conditions cannot be 0.");
            }
            setup.nc_ = nc;
            calculate_experiment_size();

            return *this;
        }

        ExperimentSetupBuilder& setNumDependentVariables(const int nd) {
            if (nd <= 0) {
                throw std::invalid_argument("Number of dependent variables cannot be 0.");
            }
            setup.nd_ = nd;
            calculate_experiment_size();

            return *this;
        }

        ExperimentSetupBuilder& setNumItems(const int ni) {
            // TODO: To be integerated!

            return *this;
        }

        ExperimentSetupBuilder& setNumObservations(const int nobs) {
            check_expr_size();

            fill_vector<int>(setup.nobs_, setup.ng_, nobs);

            return *this;
        }

        ExperimentSetupBuilder& setNumObservations(const arma::Row<int> &nobs) {
            if (!is_expr_size_decided){
                calculate_experiment_size();
            }
            if (nobs.size() != setup.ng_){
                throw std::length_error("The size of nobs does not match size of the experiment.");
            }

            // If everything ok, replace the vector with the given vector
            setup.nobs_ = nobs;

            return *this;
        }

        ExperimentSetupBuilder& setMeans(const double mean) {
            if (!is_expr_size_decided) {
                calculate_experiment_size();
            }

            fill_vector<double>(setup.means_, setup.ng_, mean);

            return *this;
        }

        ExperimentSetupBuilder& setMeans(const arma::Row<double> &means) {
            if (!is_expr_size_decided){
                calculate_experiment_size();
            }
            if (means.size() != setup.ng_){
                throw std::length_error("The size of nobs does not match size of the experiment.");
            }
            setup.means_ = means;
            return *this;
        }

        ExperimentSetupBuilder& setVariance(const double var) {
            if (!is_expr_size_decided) {
                calculate_experiment_size();
            }
            
            fill_vector<double>(setup.vars_, setup.ng_, var);

            is_vars_set = true;
            return *this;
        }

        ExperimentSetupBuilder& setVariance(const arma::Row<double> &vars) {
            if (!is_expr_size_decided){
                calculate_experiment_size();
            }
            if (vars.size() != setup.ng_){
                throw std::length_error("The size of vars does not match size of the experiment.");
            }
            setup.vars_ = vars;
            is_vars_set = true;
            return *this;
        }

        ExperimentSetupBuilder& setCovariance(const double cov) {
            if (!is_expr_size_decided) {
                calculate_experiment_size();
            }

            if (!is_vars_set) {
                throw std::invalid_argument("Please set the variance before the covariance.");
            }else{
                setup.sigma_ = setup.constructCovMatrix(setup.vars_, cov);
            }

            return *this;
        }

        ExperimentSetupBuilder& setCovarianceMatrix(const arma::Mat<double> &sigma) {
            if (!is_expr_size_decided){
                calculate_experiment_size();
            }
            
            setup.vars_ = arma::Row<double>(setup.ng_);
            setup.sigma_ = arma::Mat<double>(setup.ng_, setup.ng_);

            if (arma::size(setup.sigma_) != arma::size(sigma)){
               throw std::length_error("The size of covs does not match size of the experiment.");
            }
            
            setup.vars_ = sigma.diag().t();
            setup.sigma_ = sigma;

            return *this;
        }
        
        ExperimentSetupBuilder& setTestStrategy(const TestStrategy::TestStrategyParameters &tsp) {
            setup.tsp_ = tsp;
            return *this;
        }
        
        ExperimentSetupBuilder& setDataStrategy(const DataStrategy::DataStrategyParameters &dsp) {
            setup.dsp_ = dsp;
            return *this;
        }

        ExperimentSetup build() const {
            return setup;
        } 

        operator ExperimentSetup() const {
            return setup;
        }



    };

}

#endif //SAMPP_EXPERIMENTSETUP_H
