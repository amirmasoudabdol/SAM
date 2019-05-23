//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENT_H
#define SAMPP_EXPERIMENT_H

#include <vector>
#include <functional>

#include "sam.h"

#include "DataStrategy.h"
#include "ExperimentSetup.h"
#include "EffectStrategy.h"
#include "TestStrategy.h"
#include "Submission.h"

namespace sam {

    class Submission;
    class EffectStrategy;

    /**
     @brief Experiment class declaration

     @note This could be an abstract class. The abstract will define the body an
     experiment while subclasses customize it for different type of experiments.
     This can be used to save space because I can only define relevant variables
     for each type of experiment
     */
    class Experiment {
        
    public:
        ExperimentSetup setup;
        std::shared_ptr<DataStrategy> data_strategy;
        std::shared_ptr<TestStrategy> test_strategy;
        std::shared_ptr<EffectStrategy> effect_strategy;

        arma::Row<int> nobs;
        arma::Row<double> means;
        arma::Row<double> vars;
        arma::Row<double> ses;
        arma::Row<double> statistics;
        arma::Row<double> pvalues;
        arma::Row<double> effects;
        arma::Row<short> sigs;
        
        std::vector<arma::Row<double> > measurements;
        
        bool latentDesign = false;
        std::vector<arma::Row<double> > items;
        arma::Row<double> latent_means;
        arma::Row<double> latent_variances;
        std::vector<std::vector<double>> latent_cov_matrix;
        
        //! A function for computing the latent variable
        //! This is rather new and I still need to implement it. It's going to
        //! use std::function, it's basically more efficient and less verbose
        //! Strategy Pattern ;P
        void latent_function(std::function<void(Experiment *)>);
        
        Experiment() = default;
        
        explicit Experiment(json& experiment_config);
        
        explicit Experiment(ExperimentSetup& e) : setup{e} {

            data_strategy = DataStrategy::build(setup.dsp_);
            test_strategy = TestStrategy::build(setup.tsp_);
            effect_strategy = EffectStrategy::build(setup.esp_);
            
            initResources(setup.ng());
        };
        
        
        Experiment(ExperimentSetup &e,
                   std::shared_ptr<DataStrategy> &ds,
                   std::shared_ptr<TestStrategy> &ts,
                   std::shared_ptr<EffectStrategy> &efs) :
        setup{e}, data_strategy{ds}, test_strategy{ts}, effect_strategy{efs}
        {
            initResources(setup.ng());
        };
        
        
        /**
         Runs the Test Strategy
         */
        void runTest();
        
        /**
         Set or re-set the Test Strategy

         @param t A reference to a Test Strategy instance
         */
        void setTestStrategy(std::shared_ptr<TestStrategy> &ts){
            test_strategy = ts;
        }

        /**
         Set or re-set the Data Strategy

         @param d A reference to a Data Strategy instance
         */
        void setDataStrategy(std::shared_ptr<DataStrategy> &ds) {
            data_strategy = ds;
        }
        

        void setEffectSizeEstimator(std::shared_ptr<EffectStrategy> &es) {
            effect_strategy = es;
        };

        // Initialize the Experiment
        void initResources(int len);
        
        
        /**
         Helper function for the Researcher to fully initilize the experiment
         by generating the data, calculating the statistics and effects, as
         well as running the test.
         */
        void initExperiment();
        
        /**
         Use the `data_strategy` to generate the data.
         
         @note The `data_strategy` takes over the entire experiment and
         populate the `measurements` based on `setup`'s parameters.
         */
        void generateData();
        
        
        /**
         Calculate the statistics by sending the `experiment` to the
         `test_strategy`.
         */
        void calculateStatistics();
        
        
        /**
         Iterates over the list of EffectSizeEstimators, and calculate different
         different estimates accordingly.
         */
        void calculateEffects();

        void recalculateEverything();
        
        void randomize();


        //! Indicates if any hacking routine has been applied on the experiment
        bool is_hacked = false;
        std::vector<int> hacks_history;
        
    };
    
}

#endif //SAMPP_EXPERIMENT_H
