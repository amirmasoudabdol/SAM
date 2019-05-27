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
#include "test_fixtures.h"
#include "Experiment.h"
#include "ExperimentSetup.h"
#include "DecisionStrategy.h"
#include "HackingStrategy.h"
#include "RandomNumberGenerator.h"

using namespace arma;
using namespace sam;
using namespace std;

bool FLAGS::VERBOSE = false;
bool FLAGS::PROGRESS = false;
bool FLAGS::DEBUG = false;
bool FLAGS::UPDATECONFIG = false;

BOOST_FIXTURE_TEST_SUITE( optional_stopping, sample_linear_experiment );

    BOOST_AUTO_TEST_CASE( constructors )
    {
        HackingStrategyParameters hsp;
        hsp.name = HackingMethod::OptionalStopping;
    }

    BOOST_AUTO_TEST_CASE( testing_measurements_size )
    {
    	// Using PatientDecision Maker because he'd satisfy the hacking strategy
        DecisionStrategy::DecisionStrategyParameters dsp;
        dsp.name = DecisionType::PatientDecisionMaker;
        dsp.preference = DecisionPreference::PreRegisteredOutcome;
        auto patient_dec_maker = DecisionStrategy::build(dsp);

        auto optional_stopping = OptionalStopping("dv", 3, 1, 1);
        
        experiment->generateData();
        experiment->calculateStatistics();

        for (int i = 0; i < linear_setup.setup.ng(); ++i)
            BOOST_TEST( experiment->measurements[i].size() == linear_setup.setup.nobs()[i]);

        optional_stopping.perform(experiment, patient_dec_maker.get());

        for (int i = 0; i < linear_setup.setup.ng(); ++i)
            BOOST_TEST( experiment->measurements[i].size() != linear_setup.setup.nobs()[i]);

    }



BOOST_AUTO_TEST_SUITE_END();

BOOST_FIXTURE_TEST_SUITE( outliers_removal, sample_linear_experiment );

    BOOST_AUTO_TEST_CASE( constructors )
    {
        HackingStrategyParameters hsp;
        hsp.name = HackingMethod::SDOutlierRemoval;

    }

    BOOST_AUTO_TEST_CASE( testing_measurements_size )
    {
    	// Using PatientDecision Maker because he'd satisfy the hacking strategy
        DecisionStrategy::DecisionStrategyParameters dsp;
        dsp.name = DecisionType::PatientDecisionMaker;
        dsp.preference = DecisionPreference::PreRegisteredOutcome;
        auto patient_dec_maker = DecisionStrategy::build(dsp);

        auto outliers_removal = SDOutlierRemoval("dv", "max first", 3, 1, 1, 20, {1});

        experiment->generateData();
        experiment->calculateStatistics();

        for (int i = 0; i < linear_setup.setup.ng(); ++i)
            BOOST_TEST( experiment->measurements[i].size() == linear_setup.setup.nobs()[i]);

        // Adding 3 very far values
        RandomNumberGenerator rng;
        for (int i = 0; i < linear_setup.setup.ng(); ++i) {
            auto outliers = rng.normal(2, 0.1, 3);
            experiment->measurements[i].insert_cols(experiment->measurements[i].size(), outliers);
        }

        // Performing the hack
        outliers_removal.perform(experiment, patient_dec_maker.get());

        // Expecting them to be removed!
        for (int i = 0; i < linear_setup.setup.ng(); ++i)
            BOOST_TEST( experiment->measurements[i].size() == linear_setup.setup.nobs()[i]);
    }

BOOST_AUTO_TEST_SUITE_END();

BOOST_FIXTURE_TEST_SUITE( groups_pooling, sample_linear_experiment );


    BOOST_AUTO_TEST_CASE( constructors )
    {
        HackingStrategyParameters hsp;
        hsp.name = HackingMethod::GroupPooling;

    }

    BOOST_AUTO_TEST_CASE( testing_measurements_size )
    {
        // Using PatientDecision Maker because he'd satisfy the hacking strategy
        DecisionStrategy::DecisionStrategyParameters dsp;
        dsp.name = DecisionType::PatientDecisionMaker;
        dsp.preference = DecisionPreference::PreRegisteredOutcome;
        auto patient_dec_maker = DecisionStrategy::build(dsp);

        auto groups_pooling = GroupPooling({2});

        experiment->generateData();
        experiment->calculateStatistics();

        for (int i = 0; i < linear_setup.setup.ng(); ++i)
            BOOST_TEST( experiment->measurements[i].size() == linear_setup.setup.nobs()[i]);

        // Performing the hack
        groups_pooling.perform(experiment, patient_dec_maker.get());

        std::cout << experiment->nobs << std::endl;

        int n_new_groups = 3;
        BOOST_TEST( experiment->measurements.size() == n_new_groups + linear_setup.setup.ng() );

        for (int i = linear_setup.setup.ng(); i < n_new_groups + linear_setup.setup.ng(); ++i)
            BOOST_TEST( experiment->measurements[i].size() == 2 * linear_setup.nobs);
    }



BOOST_AUTO_TEST_SUITE_END();