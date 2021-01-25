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
#include "ReviewStrategy.h"
#include "HackingStrategy.h"
#include "Researcher.h"
#include "Submission.h"

#include "effolkronium/random.hpp"

using namespace arma;
using namespace sam;
using namespace std;
using Random = effolkronium::random_static;

struct SampleResearch {

    int nsims = 1000;

    int nc = 2;
    int nd = 1;
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

    DataStrategy *ds;
    json d_s_conf = {
        {"name", "LinearModel"},
        {"means", {0.0, 0.2}},
        {"covs", 0.0},
        {"vars", 0.1},
        {"stddevs", 1.0}
    };
    
    TestStrategy *ts;
    json t_s_conf = {
        {"name", "TTest"},
        {"alpha", 0.05},
        {"side", "TwoSided"}
    };

    EffectStrategy *es;
    json e_s_conf = {
        {"name", "CohensD"}
    };

    ExperimentSetup setup;
    Experiment *experiment;

    Journal *journal;
    json j_conf = {
        {"name", "PLOS"},
        {"max_pubs", 70}
    };
    json s_s_conf = {
        {"name", "SignificantSelection"},
        {"alpha", 0.05},
        {"pub_bias", 0.95},
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

    ResearchStrategy *des;
    json de_s_conf = {
        {"name", "PatientDecisionMaker"},
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
        
        j_conf["review_strategy"] = s_s_conf;
        
        auto ts = TestStrategy::build(t_s_conf);

        setup = ExperimentSetup::create()
                .setNumConditions(nc)
                .setNumDependentVariables(nd)
                .setNumItems(ni)
                .setNumObservations(nobs)
//                .setMeans(mean)
//                .setVariance(var)
//                .setCovariance(cov)
                .setDataStrategyParameters(d_s_conf)
                .setTestStrategyParameters(t_s_conf)
                .setEffectStrategyParameters(e_s_conf)
                .build();

        experiment = new Experiment(setup);

//        researcher = Researcher::create("John")
//                                    .createExperiment(setup)
//                                    .createResearchStrategy(de_s_conf)
//                                    .createJournal(j_conf)
//                                    .build();
    }


    void runSampleSimulation(int nsims = 1) {
        // Initializing the csv writer
    
        for (int i = 0; i < nsims; i++) {

            while (researcher.journal->isStillAccepting()) {

                researcher.research();
                
            }

            publications.push_back(researcher.journal->publications_list);
                        
            researcher.journal->clear();
        }


    }
};

#endif //SAMPP_TEST_FIXTURES_H
