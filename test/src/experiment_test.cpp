//
// Created by Amir Masoud Abdol on 2019-04-30
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE Experiment Tests

#include <boost/test/unit_test.hpp>
namespace tt = boost::test_tools;

#include <armadillo>
#include <iostream>

#include "Experiment.h"
#include "ExperimentSetup.h"
#include "DataStrategy.h"
#include "TestStrategy.h"
#include "EffectStrategy.h"

#include "sam.h"
#include "test_fixtures.h"

using namespace arma;
using namespace sam;
using namespace std;


bool FLAGS::VERBOSE = false;
bool FLAGS::PROGRESS = false;
bool FLAGS::DEBUG = false;
bool FLAGS::UPDATECONFIG = false;

BOOST_FIXTURE_TEST_SUITE ( constructors, sample_fixed_size_linear_experiment_setup )


    BOOST_AUTO_TEST_CASE( test1 )
    {
//        ExperimentSetup ex;

    }

    BOOST_AUTO_TEST_CASE( test2 )
    {
        Experiment expr(setup);

        BOOST_TEST((expr.data_strategy->params.name == DataStrategy::DataModel::LinearModel));
        BOOST_TEST((expr.test_strategy->params.name == TestStrategy::TestMethod::TTest));
        BOOST_TEST((expr.effect_strategy->params.name == EffectStrategy::EffectEstimator::CohensD));

    }

	BOOST_AUTO_TEST_CASE( linear_experiment )
	{
        auto ts = TestStrategy::build(tsp);

        auto ds = DataStrategy::build(dsp);

	  	auto es = EffectStrategy::build(esp);

	    Experiment expr(setup, ds, ts, es);

	    BOOST_TEST(expr.setup.ng() == ng);

	}


	BOOST_AUTO_TEST_CASE( linear_data_strategy_testing_stats )
	{

        // srand(42);

	    auto ts = TestStrategy::build(tsp);

	    auto ds = DataStrategy::build(dsp);

        auto es = EffectStrategy::build(esp);

        Experiment expr(setup, ds, ts, es);

	    expr.generateData();
	    expr.calculateStatistics();

	    BOOST_TEST(expr.measurements.size() == ng);

	    BOOST_TEST(expr.means.size() == ng);

        // Checking means ---------------------------------
	    for (int i = 0; i < expr.setup.ng(); ++i)
	        BOOST_CHECK_SMALL(expr.means[i] - v_means[i], 0.1);

        // Checking variance ------------------------------
        for (int i = 0; i < expr.setup.ng(); ++i)
            BOOST_CHECK_SMALL(expr.vars[i] - v_vars[i], 0.1);

        // Checking covariance ----------------------------
        arma::mat dt(nobs, ng);
        for (int i = 0; i < expr.setup.ng(); ++i)
            dt.col(i) = expr.measurements[i].as_col();

        arma::mat corr = arma::cor(dt);
        for (int i = 0; i < setup.ng(); ++i)
            for (int j = 0; j < setup.ng(); ++j)
                BOOST_CHECK_SMALL(corr(i, j) - v_sigma(i, j), 0.1);

	}


BOOST_AUTO_TEST_SUITE_END()
