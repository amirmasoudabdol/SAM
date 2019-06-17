//
// Created by Amir Masoud Abdol on 2019-06-17.
//

#define BOOST_TEST_MODULE MetaAnalysis Test
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <armadillo>
#include <iostream>
#include <fstream>

#include "ExperimentSetup.h"
#include "Experiment.h"
#include "Researcher.h"
#include "HackingStrategy.h"
#include "Journal.h"
#include "DecisionStrategy.h"
#include "PersistenceManager.h"

#include "sam.h"

#include "test_fixtures.h"

using namespace arma;
using namespace sam;
using namespace std;

using json = nlohmann::json;

bool FLAGS::VERBOSE = false;
bool FLAGS::PROGRESS = false;
bool FLAGS::DEBUG = false;
bool FLAGS::UPDATECONFIG = false;

BOOST_FIXTURE_TEST_SUITE( meta_analysis_strategy, SampleResearch )

    BOOST_AUTO_TEST_CASE( fixed_effect_model )
    {
        
        double r_estimate = 0.7766;
        
        nobs = 20;
        mean = 0.5;
        var = 2.;
        cov = 0.75;
        
        j_conf["max_pubs"] = 20;
        
        initResearch();
        
        runSampleSimulation();
        
        PersistenceManager::Writer pubwriter("/Users/amabdol/meta_pubs.csv");
        
        FixedEffectEstimator fixed_model;
        std::cout << "Fixed: " << std::endl;
        std::cout << fixed_model.estimate(publications[0]);
        
        auto fe = fixed_model.estimate(publications[0]);
        
        BOOST_CHECK_SMALL(fe[0] - r_estimate, 0.001);
        
        RandomEffectEstimator random_model;
        std::cout << "Random: " << std::endl;
        std::cout << random_model.estimate(publications[0]);
        
        pubwriter.write(publications[0]);
        
        
        
    }

BOOST_AUTO_TEST_SUITE_END()
