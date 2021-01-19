//
// Created by Amir Masoud Abdol on 2019-04-30
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE ExperimentSetup Tests

#include "nlohmann/json.hpp"
#include <boost/test/unit_test.hpp>
namespace tt = boost::test_tools;
namespace utf = boost::unit_test;

#include "Policy.h"
#include "Experiment.h"

#include "policies_variables_and_functions.h"

using json = nlohmann::ordered_json;

using namespace arma;
using namespace sam;
using namespace std;

BOOST_AUTO_TEST_SUITE( Policy )

BOOST_FIXTURE_TEST_SUITE( constructor, PoliciesVariablesAndFunctions)

BOOST_AUTO_TEST_CASE( simple_constructor ) {
  
  for (auto &var : binary_variables) {
    for (auto &op : binary_operators) {
      BOOST_TEST(true);
    }
  }
  
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( PolicyChain )


BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( PolicyChainSet )


BOOST_AUTO_TEST_SUITE_END()
