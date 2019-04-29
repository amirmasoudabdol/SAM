//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENT_H
#define SAMPP_EXPERIMENT_H

#include <vector>
#include <armadillo>

#include "DataStrategy.h"
#include "ExperimentSetup.h"
#include "EffectEstimators.h"
#include "TestStrategy.h"

namespace sam {

    class EffectSizeEstimator;

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

        arma::Row<int> nobs;
        arma::Row<double> means;
        arma::Row<double> vars;
        arma::Row<double> ses;
        arma::Row<double> statistics;
        arma::Row<double> pvalues;
        std::map<std::string, arma::Row<double>> effects;
        arma::Row<short> sigs;
        
        std::vector<arma::Row<double> > measurements;
        
        bool latentDesign = false;
        std::vector<arma::Row<double> > items;
        arma::Row<double> latent_means;
        arma::Row<double> latent_variances;
    //    std::vector<std::vector<double>> latent_cov_matrix;

        ~Experiment() {
            // TODO: Use shared_ptr<> to make sure that I don't have
            // to do cleanup
        };
        
        Experiment(json& config);
        
        Experiment(ExperimentSetup& e) : setup(e) { };

        
        void runTest();
        
        TestStrategy* test_strategy;
        
        void setTestStrategy(TestStrategy *t){
            test_strategy = t;
        }

        DataStrategy* data_strategy;
        
        void setDataStrategy(DataStrategy* d) {
            data_strategy = d;
        }
        
        std::vector<EffectSizeEstimator *> effect_size_estimators;
        void setEffectSizeEstimator(EffectSizeEstimator *efs);

        // Initialize the Experiment/**/
        void initResources(int len);
        void initExperiment();
        void generateData();
        void calculateStatistics();
        void calculateEffects();

        void recalculateEverything();
        
        void randomize();


        //! Indicates if any hacking routine has been applied on the experiment
        bool is_hacked = false;
        std::vector<int> hacks_history;
        

        
    private:




    };
    
}

#endif //SAMPP_EXPERIMENT_H
