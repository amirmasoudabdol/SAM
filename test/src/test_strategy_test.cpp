//
// Created by Amir Masoud Abdol on 2019-03-12.
//


#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE TestStrategy Tests

#include <boost/test/unit_test.hpp>

#include <armadillo>
#include <iostream>
#include "TestStrategy.h"

using namespace arma;

// most frequently you implement test cases as a free functions with automatic registration
BOOST_AUTO_TEST_CASE( r_welch_t_test )
{
    // t.test> t.test(1:10, y = c(7:20))      # P = .00001855
    double r_p_value = 0.00001855;
    
    arma::rowvec A = arma::regspace<arma::rowvec>(1, 10);
    arma::rowvec B = arma::regspace<arma::rowvec>(7, 20);
    
    TestResult res = t_test(A, B);
    
    BOOST_CHECK_SMALL(res.pvalue - r_p_value, 0.001);
}
