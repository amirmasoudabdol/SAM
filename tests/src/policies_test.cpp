//
// Created by Amir Masoud Abdol on 2019-04-30
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE PoliciesTest Tests

#include "nlohmann/json.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
namespace tt = boost::test_tools;
namespace utf = boost::unit_test;
namespace bdata = boost::unit_test::data;

#include "Policy.h"
#include "Experiment.h"

#include "policies_variables_and_functions.h"

using json = nlohmann::ordered_json;

using namespace arma;
using namespace sam;
using namespace std;

BOOST_AUTO_TEST_SUITE( PolicyTests )

BOOST_FIXTURE_TEST_SUITE( constructor, PoliciesVariablesAndFunctions)

BOOST_AUTO_TEST_CASE( simple_constructor ) {
  
  for (auto &var : quantitative_variables) {
    for (auto &op : binary_operators) {
      std::string def{var + " " + op + " 0"};
      BOOST_TEST_MESSAGE("Testing the construction of: [" + def + "] policy...");
      BOOST_CHECK_NO_THROW(Policy(def, lua));
    }
    
    for (auto &op : unary_functions) {
      std::string def{op + "(" + var + ")"};
      BOOST_TEST_MESSAGE("Testing the construction of: [" + def + "] policy...");
      BOOST_CHECK_NO_THROW(Policy(def, lua));
    }
  }
  
  for (auto &var : meta_variables) {
    std::string def{var};
    BOOST_TEST_MESSAGE("Testing the construction of: [" + def + "] policy...");
    BOOST_CHECK_NO_THROW(Policy(def, lua));
    BOOST_CHECK_NO_THROW(Policy("!" + def, lua));
  }
  
}

BOOST_AUTO_TEST_SUITE_END() // FIXTURE

  BOOST_AUTO_TEST_SUITE( call_operator_test )

    BOOST_DATA_TEST_CASE_F(PoliciesVariablesAndFunctions,
                           statistical_queries,
                           bdata::random( (bdata::distribution=std::uniform_real_distribution<float>(1, 4))) ^
                           bdata::random( (bdata::distribution=std::uniform_int_distribution<int>(100, 1000))) ^
                           bdata::random( (bdata::distribution=std::bernoulli_distribution(0.5))) ^
                           bdata::xrange(1, 4),
                           mean, nobs, status, i) {
      
      arma::rowvec data(nobs);
      data.randn(); data += mean;
      
      DependentVariable dv{data};
      Submission sub{0, 0, 0, 0, dv};
      
      std::string policy_def;
      
      policy_def = "mean < " + std::to_string(data.max());
      Policy lesser_mean{policy_def, lua};
      BOOST_TEST(lesser_mean(sub));
      BOOST_TEST(lesser_mean(dv));
      
      policy_def = "mean > " + std::to_string(data.min());
      Policy greater_mean{policy_def, lua};
      BOOST_TEST(greater_mean(sub));
      BOOST_TEST(greater_mean(dv));
      
      policy_def = "mean != " + std::to_string(mean);
      Policy non_equal_mean{policy_def, lua};
      BOOST_TEST(non_equal_mean(sub));
      BOOST_TEST(non_equal_mean(dv));
      
      policy_def = "nobs < " + std::to_string(nobs);
      Policy lesser_nobs{policy_def, lua};
      BOOST_TEST(not lesser_nobs(sub));
      BOOST_TEST(not lesser_nobs(dv));
      
      policy_def = "nobs <= " + std::to_string(nobs);
      Policy lesser_eq_nobs{policy_def, lua};
      BOOST_TEST(lesser_eq_nobs(sub));
      BOOST_TEST(lesser_eq_nobs(dv));
      
      policy_def = "nobs >= " + std::to_string(nobs);
      Policy greater_eq_nobs{policy_def, lua};
      BOOST_TEST(greater_eq_nobs(sub));
      BOOST_TEST(greater_eq_nobs(dv));
      
      policy_def = "nobs == " + std::to_string(nobs);
      Policy equal_nobs{policy_def, lua};
      BOOST_TEST(equal_nobs(sub));
      BOOST_TEST(equal_nobs(dv));
      
      
      // Testing the Status
      // ------------------
      
      dv.setHackedStatus(status);
      BOOST_TEST(dv.isHacked() == status);

      policy_def = "hacked";
      Policy is_hacked{policy_def, lua};
      BOOST_TEST(is_hacked(dv) == status);

      dv.setCandidateStatus(status);
      BOOST_TEST(dv.isCandidate() == status);

      policy_def = "candidate";
      Policy is_candidate{policy_def, lua};
      BOOST_TEST(is_candidate(dv) == status);
      
      // On Submission
      Submission s2{0, 1, 2, 3, dv};
      BOOST_TEST(s2.isHacked() == status);
      BOOST_TEST(s2.isCandidate() == status);
      BOOST_TEST(is_hacked(s2) == status);
      BOOST_TEST(is_candidate(s2) == status);
      
    }


  BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END() // SUITE


BOOST_AUTO_TEST_SUITE( PolicyChainTests )


BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( PolicyChainSetTests )


BOOST_AUTO_TEST_SUITE_END()
