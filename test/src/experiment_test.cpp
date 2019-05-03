//
// Created by Amir Masoud Abdol on 2019-04-30
//


#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE ExperimentSetup Tests

#include <boost/test/unit_test.hpp>

#include <armadillo>
#include <iostream>

#include "Experiment.h"
#include "ExperimentSetup.h"
#include "TestStrategy.h"
#include "DataStrategy.h"

#include "sam.h"

using namespace arma;
using namespace sam;

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
                          20, .147, 1.0, 0.1,
                          test_params, data_params);


    auto ts = TestStrategy::build(es);

    auto ds = DataStrategy::build(es);

    
    Experiment exp(es, ds, ts);

    BOOST_TEST(exp.setup.ng() == ng);


}