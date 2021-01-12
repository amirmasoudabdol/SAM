//
// Created by Amir Masoud Abdol on 2019-04-30
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE ExperimentSetup Tests

#include <boost/test/unit_test.hpp>
namespace tt = boost::test_tools;
namespace utf = boost::unit_test;

#include <algorithm>

#include "sam.h"
#include "ExperimentSetup.h"

#include "test_fixtures.h"

using namespace arma;
using namespace sam;
using namespace std;

BOOST_AUTO_TEST_CASE( default_constructor, * utf::enabled() )
{
    ExperimentSetup setup;

    BOOST_TEST(true);
}

BOOST_FIXTURE_TEST_SUITE( experiment_setup_builder, SampleResearch )


    BOOST_AUTO_TEST_CASE( constructing_from_json, * utf::enabled() )
    {
      json config;
      std::ifstream configFile("/Users/amabdol/Projects/SAMpp/test/configs/simple_experiment_setup.json");
      configFile >> config;

      ExperimentSetup setup{config["experiment_parameters"]};

//      BOOST_TEST( setup.nobs() == v_nobs, tt::per_element());

      BOOST_TEST( true );

//      BOOST_TEST( setup.means() == v_means, tt::per_element());
//
//      BOOST_TEST( setup.vars() == v_vars, tt::per_element());
//
//      BOOST_TEST( setup.sigma() == v_sigma, tt::per_element());

    }


//    BOOST_AUTO_TEST_CASE( building_with_fixed_params )
//    {
//
//        ExperimentSetup setup = ExperimentSetup::create().setNumConditions(nc)
//                                .setNumDependentVariables(nd)
//                                .setNumItems(ni)
//                                .setNumObservations(nobs)
//                                .setMeans(mean)
//                                .setVariance(var)
//                                .setCovariance(cov)
//                                .build();
//
//        BOOST_TEST( setup.nobs() == v_nobs, tt::per_element());
//
//        BOOST_TEST( setup.means() == v_means, tt::per_element());
//
//        BOOST_TEST( setup.vars() == v_vars, tt::per_element());
//
//        BOOST_TEST( setup.sigma() == v_sigma, tt::per_element());
//
//    }
//
//    BOOST_AUTO_TEST_CASE( building_with_arrays )
//    {
//
//        ExperimentSetup setup = ExperimentSetup::create()
//                                                .setNumConditions(nc)
//                                                .setNumDependentVariables(nd)
//                                                .setNumItems(ni)
//                                                .setNumObservations(v_nobs)
//                                                .setMeans(v_means)
//                                                .setCovarianceMatrix(v_sigma)
//                                                .build();
//
//
//        BOOST_TEST( setup.nobs() == v_nobs, tt::per_element());
//
//        BOOST_TEST( setup.means() == v_means, tt::per_element());
//
//        BOOST_TEST( setup.vars() == v_vars, tt::per_element());
//
//        BOOST_TEST( setup.sigma() == v_sigma, tt::per_element());
//    }
//
//    BOOST_AUTO_TEST_CASE( builder_test_and_data_strategy )
//    {
//        ExperimentSetup setup = ExperimentSetup::create()
//                .setNumConditions(nc)
//                .setNumDependentVariables(nd)
//                .setNumItems(ni)
//                .setNumObservations(v_nobs)
//                .setMeans(v_means)
//                .setCovarianceMatrix(v_sigma)
//                .setTestStrategyParameters(t_s_conf)
//                .setDataStrategyParameters(d_s_conf)
//                .setEffectStrategyParameters(e_s_conf)
//                .build();
//
//        BOOST_TEST( (setup.dsp_.name == DataStrategy::DataModel::LinearModel ));
//
//    }

BOOST_AUTO_TEST_SUITE_END()

// Write some tests for randomization




