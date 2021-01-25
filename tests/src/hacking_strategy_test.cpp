//
// Created by Amir Masoud Abdol on 2019-05-03.
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE HackingStrategy Tests

#include <boost/test/unit_test.hpp>
namespace tt = boost::test_tools;

#include <armadillo>
#include <iostream>
#include <vector>
#include <algorithm>


#include "sam.h"
#include "Experiment.h"
#include "ExperimentSetup.h"
#include "ResearchStrategy.h"
#include "HackingStrategy.h"

#include "test_fixtures.h"

using namespace arma;
using namespace sam;
using namespace std;

bool FLAGS::VERBOSE = false;
bool FLAGS::PROGRESS = false;
bool FLAGS::DEBUG = false;
bool FLAGS::UPDATECONFIG = false;

BOOST_FIXTURE_TEST_SUITE( optional_stopping, SampleResearch );

//    BOOST_AUTO_TEST_CASE( constructors )
//    {
//        OptionalStopping::Parameters hsp;
//        hsp.name = HackingMethod::OptionalStopping;
//    }

    BOOST_AUTO_TEST_CASE( testing_measurements_size )
    {
    	// Using PatientDecision Maker because he'd satisfy the hacking strategy
        
        initResearch();
        
        de_s_conf["preference"] = "PreRegisteredOutcome";
        auto patient_dec_maker = ResearchStrategy::build(de_s_conf);

        auto optional_stopping = OptionalStopping("dv", 3, 1, 1);
        
        experiment->generateData();
        experiment->calculateStatistics();

        for (int i = 0; i < setup.ng(); ++i)
            BOOST_TEST( experiment->measurements[i].size() == setup.nobs()[i]);

        optional_stopping.perform(experiment, patient_dec_maker.get());

        for (int i = 0; i < setup.ng(); ++i)
            BOOST_TEST( experiment->measurements[i].size() != setup.nobs()[i]);

    }


    BOOST_AUTO_TEST_CASE ( testing_updated_statistics )
    {
        
    }


BOOST_AUTO_TEST_SUITE_END();




BOOST_FIXTURE_TEST_SUITE( outliers_removal, SampleResearch );

//    BOOST_AUTO_TEST_CASE( constructors )
//    {
//        HackingStrategyParameters hsp;
//        hsp.name = HackingMethod::OutliersRemoval;
//
//    }

    BOOST_AUTO_TEST_CASE( testing_measurements_size )
    {
        initResearch();

        de_s_conf["preference"] = "PreRegisteredOutcome";
        auto patient_dec_maker = ResearchStrategy::build(de_s_conf);

        auto outliers_removal = OutliersRemoval("dv", "max first", 3, 1, 1, 20, {1});

        experiment->generateData();
        experiment->calculateStatistics();

        for (int i = 0; i < setup.ng(); ++i)
            BOOST_TEST( experiment->measurements[i].size() == setup.nobs()[i]);

        // Adding 3 very far values
//        RandomNumberGenerator rng;

        arma::rowvec outliers(3);
        for (int i = 0; i < setup.ng(); ++i) {
            
            // adding three new outliers
            outliers.each_col([](arma::vec &v){
                v = Random::get<std::normal_distribution<double>>(2.0, 0.1);
            });
            
            experiment->measurements[i].insert_cols(experiment->measurements[i].size(), outliers);
        }

        // Performing the hack
        outliers_removal.perform(experiment, patient_dec_maker.get());

        // Expecting them to be removed!
        for (int i = 0; i < setup.ng(); ++i)
            BOOST_TEST( experiment->measurements[i].size() == setup.nobs()[i]);
    }

BOOST_AUTO_TEST_SUITE_END();

BOOST_FIXTURE_TEST_SUITE( groups_pooling, SampleResearch );


//    BOOST_AUTO_TEST_CASE( constructors )
//    {
//        HackingStrategyParameters hsp;
//        hsp.name = HackingMethod::GroupPooling;
//
//    }

    BOOST_AUTO_TEST_CASE( testing_measurements_size )
    {
        // Using PatientDecision Maker because he'd satisfy the hacking strategy
        initResearch();
        
        de_s_conf["preference"] = "PreRegisteredOutcome";
        auto patient_dec_maker = ResearchStrategy::build(de_s_conf);

        auto groups_pooling = GroupPooling({2});

        experiment->generateData();
        experiment->calculateStatistics();

        for (int i = 0; i < setup.ng(); ++i)
            BOOST_TEST( experiment->measurements[i].size() == setup.nobs()[i]);

        // Performing the hack
        groups_pooling.perform(experiment, patient_dec_maker.get());

        int n_new_groups = 3;
        BOOST_TEST( experiment->measurements.size() == n_new_groups + setup.ng() );

        for (int i = setup.ng(); i < n_new_groups + setup.ng(); ++i)
            BOOST_TEST( experiment->measurements[i].size() == 2 * nobs);
    }



BOOST_AUTO_TEST_SUITE_END();
