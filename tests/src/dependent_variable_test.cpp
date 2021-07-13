//
// Created by Amir Masoud Abdol on 2021-01-18
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE ExperimentSetup Tests

#include "nlohmann/json.hpp"
#include <boost/test/unit_test.hpp>
namespace tt = boost::test_tools;
namespace utf = boost::unit_test;

#include "DependentVariable.h"

#include "sample_experiment_setup.h"

using json = nlohmann::ordered_json;

using namespace arma;
using namespace sam;
using namespace std;


BOOST_AUTO_TEST_SUITE(constructor)

  BOOST_AUTO_TEST_CASE( from_an_arma_array ) {
    
    BOOST_TEST_MESSAGE("Testing the Initialization from arma::Row<float>...");
    
    arma::Row<float> data(100);
    data.randn();
    
    DependentVariable dp{data};
    
    BOOST_TEST(arma::approx_equal(data, dp.measurements(), "absdiff", 0.001));
    
    // Testing whether stats are initialized, and updated
    BOOST_TEST(dp.mean_ != 0);
    BOOST_TEST(dp.var_ != 0);
    
  }

  BOOST_AUTO_TEST_CASE( copy_constructor ) {
    
    BOOST_TEST_MESSAGE("Testing the Copy Constructor...");
    
    arma::Row<float> data(100);
    data.randn();
    
    DependentVariable dv{data};
    
    DependentVariable dv_copy{dv};
    BOOST_TEST(dv.mean_ == dv_copy.mean_);
    BOOST_TEST(arma::approx_equal(dv.measurements(), dv_copy.measurements(),
                                  "absdiff", 0.001));
    
  }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( working_with_dependent_variable )

  BOOST_AUTO_TEST_CASE( manipulating_measurements ) {
    
    arma::Row<float> data(100);
    data.randn();
    
    DependentVariable dp{data};
    
    dp.removeMeasurements(arma::uvec{1, 5, 10, 50});
    BOOST_TEST(dp.mean_ != arma::mean(data));
    BOOST_TEST(dp.var_ != arma::var(data));
    BOOST_TEST(dp.nobs_ == 96);
    BOOST_TEST(dp.n_removed_obs == 4);
    
    dp.addNewMeasurements(arma::Row<float>{50000});
    BOOST_TEST(dp.nobs_ == 97);
    BOOST_TEST(dp.mean_ > arma::mean(data));
    BOOST_TEST(dp.var_ > arma::var(data));
    BOOST_TEST(dp.n_added_obs == 1);
    
    BOOST_TEST(dp.true_nobs_ == 100);
  }

  BOOST_AUTO_TEST_CASE( indices_operator_test, * utf::expected_failures(1) ) {
    
    arma::Row<float> data(100);
    data.randn();
    
    DependentVariable dp{data};
    
    dp[50] = 50;
    BOOST_TEST(dp[50] == 50);
  }

  BOOST_AUTO_TEST_CASE( manipulating_dps_status) {
    
    arma::Row<float> data(100);
    data.randn();
    
    DependentVariable dp{data};
    
    dp.removeMeasurements(arma::uvec{1, 5, 10, 25, 50});
    BOOST_TEST(dp.isModified());
    BOOST_TEST(not dp.isHacked());
    
    dp.setHackedStatus(true);
    BOOST_TEST(dp.isHacked());
    BOOST_TEST(dp.isModified());
    
    dp.setCandidateStatus(true);
    BOOST_TEST(dp.isCandidate());
  }

BOOST_AUTO_TEST_SUITE_END()
