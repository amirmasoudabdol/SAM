//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_DATAGENSTRATEGY_H
#define SAMPP_DATAGENSTRATEGY_H

#include <vector>

#include "sam.h"

#include "Utilities.h"
#include "ExperimentSetup.h"

//#include "utils/mvnorm_distribution.h"

#include "mvrandom.hpp"

#include "nlohmann/json.hpp"
#include "effolkronium/random.hpp"

using Random = effolkronium::random_static;

namespace sam {
    
    using json = nlohmann::json;
    
    class Experiment;
    class ExperimentSetup;

    /**
     @brief Abstract class for Data Strategies

     A DataGenStrategy should at least two methods, `genData` and
     `genNewObservationForAllGroups`. The former is mainly used to populate a
     new Experiment while the latter is being used by some hacking strategies,
     e.g. OptionalStopping, where new data — from the same population — is
     needed.

     @note Each Data Strategy should have access to an instance of
     RandomNumberGenerator. This is usually done by creating a desired
     _random engine_ and passing the pointer to the DataGenStrategy.
     */
    class DataStrategy {

    public:
        

        enum class DataModel {
            LinearModel,
            LatentModel,
            GradedResponseModel
        };

        struct DataStrategyParameters {
            DataModel name;
        };

        DataStrategyParameters params;
        
        /**
         Factory method for DataStrategy.

         @param setup An instance of ExperimentSetup
         @return a new DataStrategy
         */

        static std::unique_ptr<DataStrategy> build(json &data_strategy_config);
        
        /**
         Pure deconstructor of the DataStrategy abstract class.
         */
        virtual ~DataStrategy() = 0;
        

        /**
         Read a CSV file and load the data into the measurement. Each column
         is considered to be one group, based on the information already
         provided in the `experiment.setup`.

         @param expr A pointer to the experiment
         @param filename The CSV filename
         */
        void loadRawData(Experiment *expr, const std::string &filename);
        
        /**
         Populates the `experiment->measurements` with data based on the parameters
         specified in `setup`.

         @param experiment A pointer to an Experiment object
         */
        virtual void
        genData(Experiment* experiment) = 0;
        
        
        /**
         Generates `n_new_obs` new observations to each group.
         
         @note This routine uses the secondary random number stream to avoid
         conflicting with the main random engine.

         @param experiment The pointer to the current experiment
         @param n_new_obs The number of new observations to be added
         @return An array of new observations
         */
        virtual std::vector<arma::Row<double> >
        genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs) = 0;
        
        
        /**
         Generate `n_new_obs` new observations for `g` group.

         @param experiment The pointer to the experiment
         @param g The target group
         @param n_new_obs The number of new observations
         @return An array of new observations
         */
        virtual arma::Row<double>
        genNewObservationsFor(Experiment* experiment, int g, int n_new_obs) = 0;
        
        
    };

    /**
     The fixed-effect data strategy will produce data from a fixed-effect
     model with the given \f$\mu\f$ and \f$\sigma\f$.
     */
    class LinearModelStrategy : public DataStrategy {

    public:
        
        struct Parameters {
            DataModel name {DataModel::LinearModel};
            
            double mean{0};
            arma::Row<double> means;
            
            double stddev{0};
            arma::Row<double> stddevs;
            
            double cov{0};
            arma::Row<double> covs;
            arma::Mat<double> sigma;
            
            Parameters() = default;
        };

        LinearModelStrategy() { };
        
        LinearModelStrategy(const Parameters p) : params(p) {
            mdist = mvrandom::mvnorm_distribution<>(params.means.t(), params.sigma);
        };
        
        LinearModelStrategy(ExperimentSetup &setup) {
//            mdist = mvrandom::mvnorm_distribution<>(setup.means().t(), setup.sigma());

            
            // Just in case...
            
        }
        
        void
        genData(Experiment* experiment);
        
        std::vector<arma::Row<double> >
        genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs);
        
        arma::Row<double>
        genNewObservationsFor(Experiment* experiment, int g, int n_new_obs);
        
    private:
        
        Parameters params;
        
        Distribution dist;
        MultivariateDistribution mdist;
    };


            // JSON Parser for LinearModelStrategy::Parameters
            inline
            void to_json(json& j, const LinearModelStrategy::Parameters& p) {
                j = json{
                    {"_name", p.name},
                    {"means", arma::conv_to<std::vector<double>>::from(p.means)},
                    {"stddevs", arma::conv_to<std::vector<double>>::from(p.stddevs)},
                    {"covs", arma::conv_to<std::vector<double>>::from(p.covs)}
                };
            }
        
            inline
            void from_json(const json& j, LinearModelStrategy::Parameters& p) {
                
                // Using a helper template function to handle the optional and throw if necessary.
                p.name = j.at("_name");
                
                // Size of the means vector is going to be used as a reference
                p.means = arma::conv_to<arma::Row<double>>::from(j.at("means").get<std::vector<double>>());
                int n = p.means.n_elem;
                
                auto sd = get_expr_setup_params<double>(j.at("stddevs"), n);
                p.stddevs = arma::conv_to<arma::Row<double>>::from(std::get<0>(sd));

                auto cv = get_expr_setup_params<double>(j.at("covs"), n * (n - 1) / 2);
                p.covs = arma::conv_to<arma::Row<double>>::from(std::get<0>(cv));

                p.sigma = constructCovMatrix(p.stddevs, p.covs, n);
              
            }


    //=================================================================================//


    /**
     A Data Strategy for constructing a general
     [Structural Equaiton Model](https://en.wikipedia.org/wiki/Structural_equation_modeling).

     @note LatentDataStrategy will generate individual items, therefore it
     might be slower than other models.
     */
    class LatentDataStrategy : public DataStrategy {

    public:

        LatentDataStrategy() {

        }
        
//        LatentDataStrategy(ExperimentSetup &setup) {
//            
//        }
        
        LatentDataStrategy(DataStrategyParameters dsp) {
            params = dsp;

        }
        
        void genData(Experiment* experiment);
        
        std::vector<arma::Row<double> >
        genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs);
        
        arma::Row<double>
        genNewObservationsFor(Experiment* experiment, int g, int n_new_obs);
        
    private:
        //! A function for computing the latent variable
        //! This is rather new and I still need to implement it. It's going to
        //! use std::function, it's basically more efficient and less verbose
        //! Strategy Pattern ;P
        void latent_function();
    };

    /**
     @brief Simulate data from Graded Response Model.
     @Note:
        - DVs in GRM are distinguished by their participant abilities to answer tests. Therefore, we'll
        have `ng_` number of `abilities`. This value is being used to, in each group, to initialize a normal
        distribution of `\theta ~ N(abilitis[i], 1)`.
        - \f$ \beta \f$
     */
    class GRMDataStrategy : public DataStrategy {
    public:

        struct Parameters {
            
            DataModel name;
            
            int n_items;
            int n_categories;

            arma::Row<double> difficulties;
            arma::Row<double> abilities;
            
//            Distribution difficulties_dist;
//            Distribution abilities_dist;
            
            Parameters() = default;
            
        };
        
        GRMDataStrategy() {};

        GRMDataStrategy(const Parameters &p) : params(p) {
            difficulties_dist = std::normal_distribution<>(params.difficulties.at(0), 1);
            
            /// Note: I'm a bit confused about the difficulties, and I think I'm making
            /// a mess with the `Parameters`. This works but I need some serious cleanup
            /// on the parameters.
            betas.resize(params.n_items, params.n_categories);
//            betas.imbue([&]() { return Random::get(difficulties_dist); });
//            beta.each_row( [this](arma::vec &v) {v = Random::get(})

            poa.resize(params.n_items, params.n_categories);
            responses.resize(params.n_items, params.n_categories);
            urand.resize(params.n_items, params.n_categories);
//            scores.resize(params.n_items, 1);
        };


        void
        genData(Experiment* experiment);
        
        std::vector<arma::Row<double> >
        genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs);
        
        arma::Row<double>
        genNewObservationsFor(Experiment* experiment, int g, int n_new_obs);
        
    private:
        
        Parameters params;
        
        
        //! Distribution of difficulty of items. Currently this is default to \f$ N(0, 1) \f$. Its mean
        //! can be overwritten with the diffilcitues parameter.
        //! TODO: I can replace this with a \f$ MN~(\mu, O) \f$ and have different
        //! different difficulties for different items
        Distribution difficulties_dist = std::normal_distribution<>{};
        
        //! Distribution of participant's ability to answer a test correctly. This is being used to build the POA matrix
        //! which is being used later to calculate the test scores. It's default to `\f$ N(0, 1) \f$. The mean of this
        //! distribution is used to define the difference between each group.
        //! @Note: There should be at least `ng_` means available.
        Distribution abilities_dist = std::normal_distribution<>{};
        
        // TODO: This can be replaced by `Random::get<bool>` but I need to test it first.
        Distribution uniform_dist = std::uniform_real_distribution<>{};
        
        arma::mat poa; // probablity of answering
        arma::umat responses; // responses to items, binary
//        arma::mat scores;
        arma::mat sumofscores;
        
        arma::mat urand;
        
        //! Item difficulties
        arma::mat betas;

        //! Participants abilities
        arma::mat thetas;
        
        // This can be implemented again if I need a direct access to item scores
//        arma::umat generate_binary_scores(const double theta);
        double generate_sum_of_scores(const double theta);
        
        
    };


            // JSON Parser for GRMDataStrategy::Parameters
            inline
            void to_json(json& j, const GRMDataStrategy::Parameters& p) {
                j = json{
//                    {"_name", magic_enum::enum_name<DataStrategy::DataModel>(p.name)},
                    {"_name", p.name},
                    {"n_items", p.n_items},
                    {"n_categories", p.n_categories},
                    {"difficulties", arma::conv_to<std::vector<double>>::from(p.difficulties)},
                    {"abilities", arma::conv_to<std::vector<double>>::from(p.abilities)}
                };
            }
        
            inline
            void from_json(const json& j, GRMDataStrategy::Parameters& p) {
                
                // Using a helper template function to handle the optional and throw if necessary.
                p.name = j.at("_name");
                
                j.at("n_items").get_to(p.n_items);
                j.at("n_categories").get_to(p.n_categories);
                
                p.difficulties = arma::conv_to<arma::Row<double>>::from(j.at("difficulties").get<std::vector<double>>());
                p.abilities = arma::conv_to<arma::Row<double>>::from(j.at("abilities").get<std::vector<double>>());
                
            }


    NLOHMANN_JSON_SERIALIZE_ENUM( DataStrategy::DataModel, {
        {DataStrategy::DataModel::LinearModel, "LinearModel"},
        {DataStrategy::DataModel::LatentModel, "LatentModel"},
        {DataStrategy::DataModel::GradedResponseModel, "GradedResponseModel"},
    })

}

#endif //SAMPP_DATAGENSTRATEGY_H
