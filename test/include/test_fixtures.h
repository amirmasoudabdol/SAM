//
// Created by Amir Masoud Abdol on 2019-05-20.
//

#ifndef SAMPP_TEST_FIXTURES_H
#define SAMPP_TEST_FIXTURES_H

#include <iostream>

#include "nlohmann/json.hpp"

#include "sam.h"
#include "Experiment.h"
#include "ExperimentSetup.h"
#include "DataStrategy.h"
#include "TestStrategy.h"
#include "EffectStrategy.h"
#include "Journal.h"
#include "SelectionStrategy.h"
#include "HackingStrategy.h"
#include "Researcher.h"

using namespace arma;
using namespace sam;
using namespace std;
using json = nlohmann::json;

//struct sample_fixed_size_linear_experiment_setup {
//
//    int nsim = 1000;
//
//    int nc = 2;
//    int nd = 3;
//    int ni = 0;
//    int ng = nc * nd;
//
//    int nobs = 20;
//    double mean = 0.25;
//    double var = 1;
//    double cov = 0.00;
//
//    arma::Row<int> v_nobs;
//    arma::Row<double> v_means;
//    arma::Row<double> v_vars;
//    arma::Mat<double> v_sigma;
//
//    DataStrategy::DataStrategyParameters dsp;
//
//    TestStrategy::TestStrategyParameters tsp;
//
//    EffectStrategy::EffectStrategyParameters esp;
//
//    // string ds_name = "LinearModel";
//
//    ExperimentSetup setup;
//
//    sample_fixed_size_linear_experiment_setup() {
//
//        BOOST_TEST_MESSAGE( "setup the test experiment" );
//
//        v_nobs = arma::Row<int>(ng).fill(nobs);
//        v_means = arma::Row<double>(ng).fill(mean);
//        v_vars = arma::Row<double>(ng).fill(var);
//        v_sigma = arma::Mat<double>(ng, ng).fill(cov);
//        v_sigma.diag() = v_vars;
//
//        dsp.name = DataStrategy::DataModel::LinearModel;
//        dsp.seed1 = 42;
//        dsp.seed2 = 7;
//
//        tsp.name = TestStrategy::TestMethod::TTest;
//        tsp.side = TestStrategy::TestSide::TwoSided;
//        tsp.alpha = 0.05;
//
//        esp.name = EffectStrategy::EffectEstimator::CohensD;
//
//        setup = ExperimentSetup::create()
//                .setNumConditions(nc)
//                .setNumDependentVariables(nd)
//                .setNumItems(ni)
//                .setNumObservations(nobs)
//                .setMeans(mean)
//                .setVariance(var)
//                .setCovariance(cov)
//                .setDataStrategy(dsp)
//                .setTestStrategy(tsp)
//                .setEffectStrategy(esp)
//                .build();
//
//    }
//};


//struct sample_linear_experiment {
//    sample_fixed_size_linear_experiment_setup linear_setup;
//
//    // ExperimentSetup setup;
//    Experiment *experiment;
//
//    sample_linear_experiment () {
//        // setup = linear_setup.setup;
//        experiment = new Experiment(linear_setup.setup);
//
//    }
//};


//struct sample_journal {
//    Journal *journal;
//    Journal::JournalParameters jp;
//    SelectionStrategy::SelectionStrategyParameters ssp;
//
//    sample_journal() {
//
//        jp.name = "PNAS";
//        jp.max_pubs = 70;
//
//        ssp.name = SelectionMethod::SignificantSelection;
//        ssp.alpha = 0.05;
//        ssp.pub_bias = 0.95;
//        ssp.side = 1;
//
//        journal = new Journal(jp, ssp);
//    }
//};

struct SampleResearch {

    int nsims = 1000;

    int nc = 2;
    int nd = 3;
    int ni = 0;
    int ng = nc * nd;

    int nobs = 20;
    double mean = 0;
    double var = 1;
    double cov = 0.00;

    arma::Row<int> v_nobs;
    arma::Row<double> v_means;
    arma::Row<double> v_vars;
    arma::Mat<double> v_sigma;

    DataStrategy::DataStrategyParameters dsp;
    DataStrategy *ds;
    json d_s_conf = {
        {"name", "LinearModel"}
    };
    
    TestStrategy *ts;
    json t_s_conf = {
        {"name", "TTest"},
        {"alpha", 0.5},
        {"side", "TwoSided"}
    };

//    EffectStrategy::EffectStrategyParameters esp;
    EffectStrategy *es;
    json e_s_conf = {
        {"name", "CohensD"}
    };

    ExperimentSetup setup;
    Experiment *experiment;
    
    Journal::JournalParameters jp;
    
    
    Journal *journal;
    json j_conf = {
        {"name", "PLOS"},
        {"max-pubs", 70}
    };
    json s_s_conf = {
        {"name", "SignificantSelection"},
        {"alpha", 0.05},
        {"pub-bias", 42},
        {"side", 1}
    };
    
    HackingStrategy *hs;
    json h_s_conf = {
        {"name", "OptionalStopping"},
        {"level", "dv"},
        {"max_attempts", 10},
        {"n_attempts", 3},
        {"num", 10}
    };

    DecisionStrategy *des;
    json de_s_conf = {
        {"name", "PatientDecisionMaker"},
        {"preference", "MinPvalue"}
    };

    Researcher researcher;

    SampleResearch() {

        BOOST_TEST_MESSAGE( "Preparing the sample research enviroment..." );

        v_nobs = arma::Row<int>(ng).fill(nobs);
        v_means = arma::Row<double>(ng).fill(mean);
        v_vars = arma::Row<double>(ng).fill(var);
        v_sigma = arma::Mat<double>(ng, ng).fill(cov);
        v_sigma.diag() = v_vars;

//        dsp.name = DataStrategy::DataModel::LinearModel;
//        dsp.seed1 = 42;
//        dsp.seed2 = 7;

//        esp.name = EffectStrategy::EffectEstimator::CohensD;

    }


    void initResearch() {
        
        j_conf["selection-strategy"] = s_s_conf;
        
        auto ts = TestStrategy::build(t_s_conf);

        setup = ExperimentSetup::create()
                .setNumConditions(nc)
                .setNumDependentVariables(nd)
                .setNumItems(ni)
                .setNumObservations(nobs)
                .setMeans(mean)
                .setVariance(var)
                .setCovariance(cov)
                .setDataStrategyParameters(d_s_conf)
                .setTestStrategyParameters(t_s_conf)
                .setEffectStrategyParameters(e_s_conf)
                .build();

        researcher = Researcher::create("John")
                                    .createExperiment(setup)
                                    .createDecisionStrategy(de_s_conf)
                                    .createJournal(j_conf)
                                    .build();
    }
};

#endif //SAMPP_TEST_FIXTURES_H
