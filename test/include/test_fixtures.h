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
#include "Submission.h"

#include "effolkronium/random.hpp"

using namespace arma;
using namespace sam;
using namespace std;
using json = nlohmann::json;
using Random = effolkronium::random_static;

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
        {"_name", "LinearModel"}
    };
    
    TestStrategy *ts;
    json t_s_conf = {
        {"_name", "TTest"},
        {"alpha", 0.5},
        {"side", "TwoSided"}
    };

    EffectStrategy *es;
    json e_s_conf = {
        {"_name", "CohensD"}
    };

    ExperimentSetup setup;
    Experiment *experiment;

    Journal *journal;
    json j_conf = {
        {"_name", "PLOS"},
        {"max_pubs", 70}
    };
    json s_s_conf = {
        {"_name", "SignificantSelection"},
        {"alpha", 0.05},
        {"pub_bias", 0.95},
        {"side", 1}
    };
    
    HackingStrategy *hs;
    json h_s_conf = {
        {"_name", "OptionalStopping"},
        {"level", "dv"},
        {"max_attempts", 10},
        {"n_attempts", 3},
        {"num", 10}
    };

    DecisionStrategy *des;
    json de_s_conf = {
        {"_name", "PatientDecisionMaker"},
        {"preference", "MinPvalue"}
    };

    Researcher researcher;

    std::vector<std::vector<Submission>> publications;

    SampleResearch() {

        BOOST_TEST_MESSAGE( "Preparing the sample research enviroment..." );

        v_nobs = arma::Row<int>(ng).fill(nobs);
        v_means = arma::Row<double>(ng).fill(mean);
        v_vars = arma::Row<double>(ng).fill(var);
        v_sigma = arma::Mat<double>(ng, ng).fill(cov);
        v_sigma.diag() = v_vars;

    }


    void initResearch() {
        
        j_conf["selection_strategy"] = s_s_conf;
        
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

        experiment = new Experiment(setup);

        researcher = Researcher::create("John")
                                    .createExperiment(setup)
                                    .createDecisionStrategy(de_s_conf)
                                    .createJournal(j_conf)
                                    .build();
    }


    void runSampleSimulation(int nsims = 1) {
        // Initializing the csv writer
    
        for (int i = 0; i < nsims; i++) {

            while (researcher.journal->isStillAccepting()) {

                researcher.prepareResearch();

                researcher.performResearch();

                researcher.publishResearch();
                
            }

            publications.push_back(researcher.journal->publications_list);
                        
            researcher.journal->clear();
        }


    }
};

#endif //SAMPP_TEST_FIXTURES_H
