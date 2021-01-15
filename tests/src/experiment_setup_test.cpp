//
// Created by Amir Masoud Abdol on 2019-04-30
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE ExperimentSetup Tests

#include "nlohmann/json.hpp"
#include <boost/test/unit_test.hpp>
namespace tt = boost::test_tools;
namespace utf = boost::unit_test;

#include "ExperimentSetup.h"

#include "sample_experiment_setup.h"

using json = nlohmann::ordered_json;

using namespace arma;
using namespace sam;
using namespace std;

BOOST_AUTO_TEST_SUITE(constructor)

  BOOST_AUTO_TEST_CASE( default_constructor )
  {
      ExperimentSetup setup;

      BOOST_TEST(setup.nc() == 0);
  }

  BOOST_FIXTURE_TEST_SUITE( json_constructor, ExperimentSetupSampleConfigs )

    BOOST_AUTO_TEST_CASE( json_constructor )
    {
      ExperimentSetup setup{sample_experiment_setup["experiment_parameters"]};
      
      BOOST_TEST(setup.nc() == 2);
      BOOST_TEST(setup.nd() == 2);
      BOOST_TEST(setup.ng() == 4);
      BOOST_TEST(setup.nreps() == 5);
      
      arma::Row<int> obs{10, 10, 10, 10};
      BOOST_TEST(arma::approx_equal(setup.nobs(), obs,
                                    "absdiff", 0.0001));
      
    }

  BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()


// Setter's Tests
// --------------

BOOST_FIXTURE_TEST_SUITE( setter_tests, ExperimentSetupSampleConfigs )

  BOOST_AUTO_TEST_CASE( independent_parameters )
  {
    ExperimentSetup setup{sample_experiment_setup["experiment_parameters"]};
    
    setup.setNR(1);
    BOOST_TEST(setup.nreps() == 1);
  }

  BOOST_AUTO_TEST_CASE( dependent_parameters )
  {
    ExperimentSetup setup{sample_experiment_setup["experiment_parameters"]};
    
    setup.setNC(5);
    BOOST_TEST(setup.nc() == 5);
    BOOST_TEST(setup.ng() == 10);
    
    setup.setND(4);
    BOOST_TEST(setup.ng() == 5 * 4);
  }

  BOOST_AUTO_TEST_CASE( n_obs_parameter )
  {
    ExperimentSetup setup{sample_experiment_setup["experiment_parameters"]};
    
    arma::Row<int> obs{10, 10, 10, 10};
    BOOST_TEST(arma::approx_equal(setup.nobs(), obs,
                                  "absdiff", 0.0001));
    
    
    json normal_dist = R"(
      {
      "param": {
        "dist": "normal_distribution",
        "mean": 0,
        "stddev": 1
        }
      }
    )"_json;
    
    setup.setObs(normal_dist["param"], setup.nc(), setup.nd());
    BOOST_TEST(arma::accu(setup.nobs()) < 40);
    
    obs = {10, 20, 30, 40};
    setup.setObs(obs, 2, 2);
    BOOST_TEST(arma::accu(setup.nobs()) == 100);
    
  }

BOOST_AUTO_TEST_SUITE_END()


