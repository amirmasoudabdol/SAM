//
// Created by Amir Masoud Abdol on 2019-04-30
//


#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE ExperimentSetup Tests

#include <boost/test/unit_test.hpp>
namespace tt = boost::test_tools;

#include <armadillo>
#include <iostream>

#include "Experiment.h"
#include "ExperimentSetup.h"
#include "TestStrategy.h"
#include "DataStrategy.h"
#include "EffectEstimators.h"

#include "sam.h"

using namespace arma;
using namespace sam;
using namespace std;

BOOST_AUTO_TEST_SUITE( constructors )


    BOOST_AUTO_TEST_CASE( test1 )
    {
        ExperimentSetup ex;

    }

	BOOST_AUTO_TEST_CASE( linear_experiment )
	{

        int nc = 2;
        int nd = 3;
        int ng = nc * nd;

        TestStrategyParameters test_params;
        test_params.name = TestType::TTest;
        test_params.side = TestSide::TwoSide;
        test_params.alpha = 0.05;

        DataStrategyParameters data_params;
        data_params.name = ExperimentType::LinearModel;

        ExperimentSetup es(nc, nd,
                           20, .5, .25, 0.1,
                           test_params, data_params);


        auto ts = TestStrategy::build(es);

        auto ds = DataStrategy::build(es);

	  	vector<shared_ptr<EffectSizeEstimator>> efs;
	    efs.push_back(EffectSizeEstimator::build("CohensD"));

	    Experiment expr(es, ds, ts, efs);


	    BOOST_TEST(expr.setup.ng() == ng);

//	    BOOST_TEST(expr.setup.test_strategy_parameters_.name == TestType::TTest);
	}


	BOOST_AUTO_TEST_CASE( linear_data_strategy ) {

        srand(42);

        int nc = 2;
	    int nd = 3;
	    int ng = nc * nd;

	    TestStrategyParameters test_params;
	    test_params.name = TestType::TTest;
	    test_params.side = TestSide::TwoSide;
	    test_params.alpha = 0.05;

	    DataStrategyParameters data_params;
	    data_params.name = ExperimentType::LinearModel;

	    ExperimentSetup es(nc, nd, 
	                          20, .5, .25, 0.1,
	                          test_params, data_params);


	    auto ts = TestStrategy::build(es);

	    auto ds = DataStrategy::build(es);

	  	vector<shared_ptr<EffectSizeEstimator>> efs;
	    efs.push_back(EffectSizeEstimator::build("CohensD"));

	    Experiment expr(es, ds, ts, efs);

	    expr.generateData();
	    expr.calculateStatistics();

	    BOOST_TEST(expr.measurements.size() == ng);

	    BOOST_TEST(expr.means.size() == ng);

	    vec means{0.3230, 0.1939, 0.5688, 0.2823, 0.2731, 0.3167};

	    for (int i = 0; i < expr.means.size(); ++i)
	        BOOST_CHECK_SMALL(expr.means[i] - means[i], 0.0001);

	}


BOOST_AUTO_TEST_SUITE_END()
