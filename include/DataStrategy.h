//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_DATAGENSTRATEGY_H
#define SAMPP_DATAGENSTRATEGY_H

#include <vector>

#include "sam.h"

#include "RandomNumberGenerator.h"

#include "nlohmann/json.hpp"

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
        
        int seed1 = -1;
        int seed2 = -1;

        struct DataStrategyParameters {
            int seed1 = -1;
            int seed2 = -1;
            std::string name;
        };

        DataStrategyParameters params;
        
        /**
         Factory method for DataStrategy.

         @param setup An instance of ExperimentSetup
         @return a new DataStrategy
         */
        // static std::shared_ptr<DataStrategy> build(ExperimentSetup &setup);

        static std::shared_ptr<DataStrategy> build(const std::string &name);

        static std::shared_ptr<DataStrategy> build(const DataStrategyParameters &dsp);
        
        
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

        LinearModelStrategy() {
            seed1 = rand();
            seed2 = rand();
            
            main_rng_stream = new RandomNumberGenerator(seed1);
            sec_rng_stream = new RandomNumberGenerator(seed2);
        };
        
        LinearModelStrategy(DataStrategyParameters dsp) {

            params = dsp;

            seed1 = rand();
            seed2 = rand();
            
            main_rng_stream = new RandomNumberGenerator(seed1);
            sec_rng_stream = new RandomNumberGenerator(seed2);
        };
        
        void
        genData(Experiment* experiment);
        
        std::vector<arma::Row<double> >
        genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs);
        
        arma::Row<double>
        genNewObservationsFor(Experiment* experiment, int g, int n_new_obs);
        
    private:
//        int seed1;
//        int seed2;
        RandomNumberGenerator *main_rng_stream;
        RandomNumberGenerator *sec_rng_stream;
    };

    /**
     A Data Strategy for constructing a general
     [Structural Equaiton Model](https://en.wikipedia.org/wiki/Structural_equation_modeling).

     @note LatentDataStrategy will generate individual items, therefore it
     might be slower than other models.
     */
    class LatentDataStrategy : public DataStrategy {

    public:

        LatentDataStrategy() {
            seed1 = rand();
            seed2 = rand();
            
            main_rng_stream = new RandomNumberGenerator(seed1);
            sec_rng_stream = new RandomNumberGenerator(seed2);
        }
        
        LatentDataStrategy(DataStrategyParameters dsp) {
            params = dsp;
            seed1 = rand();
            seed2 = rand();
            
            main_rng_stream = new RandomNumberGenerator(seed1);
            sec_rng_stream = new RandomNumberGenerator(seed2);
        }
        
        void genData(Experiment* experiment);
        
        std::vector<arma::Row<double> >
        genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs);
        
        arma::Row<double>
        genNewObservationsFor(Experiment* experiment, int g, int n_new_obs);
        
    private:
//        int seed1;
//        int seed2;
        RandomNumberGenerator *main_rng_stream;
        RandomNumberGenerator *sec_rng_stream;
        
    };

}

#endif //SAMPP_DATAGENSTRATEGY_H
