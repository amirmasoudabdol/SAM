//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENT_H
#define SAMPP_EXPERIMENT_H

#include <vector>

#include "DataGenStrategy.h"
#include "ExperimentSetup.h"
#include "EffectEstimators.h"
#include "TestStrategy.h"

/**
 \brief Experiment class declaration
 
 */
class Experiment {
    
    // This could be an abstract class. The abstract will define the body of
    // an experiment while subclasses customize it for different type of experiments.
    // This can be used to save space because I can only define relevant variables
    // for each type of experiment

public:
    ExperimentSetup setup;

    std::vector<int> nobs;
    std::vector<std::vector<double>> measurements;
    std::vector<double> means;
    std::vector<double> vars;
    std::vector<double> ses;
    std::vector<double> statistics;
    std::vector<double> pvalues;
    std::vector<double> effects;
    
    bool latentDesign = false;
    std::vector<std::vector<double> > items;
    std::vector<double> latent_means;
    std::vector<double> latent_variances;
    std::vector<std::vector<double>> latent_cov_matrix;

    ~Experiment() {
        // TODO: Use shared_ptr<> to make sure that I don't have
        // to do cleanup
    };
    
    Experiment(json& config);
    
    Experiment(ExperimentSetup& e) : setup(e) { };

    TestStrategy* testStrategy;
    void setTestStrategy(TestStrategy* t);
    void runTest();

    DataGenStrategy* dataStrategy;
    void setDataStrategy(DataGenStrategy* d);

    // Initialize the Experiment/**/
    void initResources();
    void initExperiment();
    void generateData();
    void calculateStatistics();
    void calculateEffects();
    
    void randomize();


    // Hacking Meta-data
    bool isHacked = false;     ///< Indicates if any hacking routine has
                               ///< been applied on the experiment
//    std::vector<HackingStrategies> hackingHistory;
    
    
//    Experiment(const Experiment& other){
//
//    }
//    
//    Experiment& operator=(const Experiment& lhs){
//
//    };

    
private:




};

#endif //SAMPP_EXPERIMENT_H
