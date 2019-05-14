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
#include "EffectEstimators.h"

#include "sam.h"

using namespace arma;
using namespace sam;
using namespace std;

struct test_experiment {
    int nc = 2;
    int nd = 3;
    int ni = 0;
    int ng = nc * nd;

    int nobs = 25;
    double mean = 0.25;
    double var = 1;
    double cov = 0.01;

    arma::Row<int> v_nobs;
    arma::Row<double> v_means;
    arma::Row<double> v_vars;
    arma::Mat<double> v_sigma;

    DataStrategy::DataStrategyParameters dsp;

    TestStrategy::TestStrategyParameters tsp;

    string ds_name = "LinearModel";

    ExperimentSetup setup;

    test_experiment() {

        BOOST_TEST_MESSAGE( "setup the test experiment" );

        v_nobs = arma::Row<int>(ng).fill(nobs);
        v_means = arma::Row<double>(ng).fill(mean);
        v_vars = arma::Row<double>(ng).fill(var);
        v_sigma = arma::Mat<double>(ng, ng).fill(cov);
        v_sigma.diag() = v_vars;

        dsp.name = ds_name;

        tsp.name = TestStrategy::TestType::TTest;
        tsp.side = TestStrategy::TestSide::TwoSide;
        tsp.alpha = 0.05;

        setup = ExperimentSetup::create()
                .setNumConditions(nc)
                .setNumDependentVariables(nd)
                .setNumItems(ni)
                .setNumObservations(nobs)
                .setMeans(mean)
                .setVariance(var)
                .setCovariance(cov);

    }
};


BOOST_FIXTURE_TEST_SUITE ( constructors, test_experiment )


    BOOST_AUTO_TEST_CASE( test1 )
    {
        ExperimentSetup ex;

    }

	BOOST_AUTO_TEST_CASE( linear_experiment )
	{
        auto ts = TestStrategy::build(tsp);

        auto ds = DataStrategy::build(dsp);

	  	vector<shared_ptr<EffectSizeEstimator>> efs;
	    efs.push_back(EffectSizeEstimator::build("CohensD"));

	    Experiment expr(setup, ds, ts, efs);


	    BOOST_TEST(expr.setup.ng() == ng);

	    BOOST_TEST(expr.setup.test_strategy_parameters_.name == TestStrategy::TestType::TTest);
	}


	BOOST_AUTO_TEST_CASE( linear_data_strategy )
	{

        srand(42);

	    auto ts = TestStrategy::build(tsp);

	    auto ds = DataStrategy::build(dsp);

	  	vector<shared_ptr<EffectSizeEstimator>> efs;
	    efs.push_back(EffectSizeEstimator::build("CohensD"));

	    Experiment expr(setup, ds, ts, efs);

	    expr.generateData();
	    expr.calculateStatistics();

	    BOOST_TEST(expr.measurements.size() == ng);

	    BOOST_TEST(expr.means.size() == ng);

	    vec means{0.3230, 0.1939, 0.5688, 0.2823, 0.2731, 0.3167};

	    for (int i = 0; i < expr.means.size(); ++i)
	        BOOST_CHECK_SMALL(expr.means[i] - means[i], 0.0001);

	}


BOOST_AUTO_TEST_SUITE_END()
