//
// Created by Amir Masoud Abdol on 2019-03-12.
//


#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE TestStrategy Tests

#include <boost/test/unit_test.hpp>

#include <armadillo>
#include <iostream>
#include "TestStrategy.h"

#include "sam.h"
#include "test_fixtures.h"

using namespace arma;
using namespace sam;
using namespace std;

bool FLAGS::VERBOSE = false;
bool FLAGS::PROGRESS = false;
bool FLAGS::DEBUG = false;
bool FLAGS::UPDATECONFIG = false;

BOOST_AUTO_TEST_SUITE( t_test_functions )

    BOOST_AUTO_TEST_CASE( one_sample_test )
    {

        //    One Sample t-test
        //
        //    data:  a
        //    t = 19.287, df = 24, p-value = 4.077e-16
        //    alternative hypothesis: true mean is not equal to 0
        //    95 percent confidence interval:
        //    2.855905 3.540381
        //    sample estimates:
        //    mean of x
        //    3.198143

        // a <- rnorm(25, 3, 1)
        Row<double> a = {3.290169, 3.031275, 2.701008, 3.703762, 4.633237, 2.327662, 3.050368,\
        2.634829, 3.358146, 3.350406, 2.490066, 3.500233, 5.485966, 3.566797, 1.945628, 2.878948,\
        2.920190, 3.402363, 2.821342, 3.640249, 4.717522, 3.353228, 2.334823, 1.997572, 2.817784};

        double r_p_value = 4.077e-16;

        TestStrategy::TestResult res = t_test(0., 0., 0.,
                mean(a), stddev(a), a.size(),
                0.05, TestStrategy::TestSide::TwoSided, true);

        BOOST_CHECK_SMALL(res.pvalue - r_p_value, 0.001);
    }

    BOOST_AUTO_TEST_CASE( two_equal_var_test )
    {

        //    Two Sample t-test
        //
        //    data:  a and b
        //    t = 4.7673, df = 46, p-value = 1.911e-05
        //    alternative hypothesis: true difference in means is not equal to 0
        //    95 percent confidence interval:
        //    0.6868482 1.6907370
        //    sample estimates:
        //    mean of x mean of y
        //    3.198143  2.009350

        // a <- rnorm(25, 3, 1)
        Row<double> a = {3.290169, 3.031275, 2.701008, 3.703762, 4.633237, 2.327662, 3.050368, 2.634829,\
        3.358146, 3.350406, 2.490066, 3.500233, 5.485966, 3.566797, 1.945628, 2.878948, 2.920190, 3.402363,\
        2.821342, 3.640249, 4.717522, 3.353228, 2.334823, 1.997572, 2.817784};

        // b <- rnorm(23, 2, 1)
        Row<double> b = {1.8461070, 1.7434951, 2.5623408, 1.2993293, 1.7287880, 1.1736090, 2.8343341, 1.0222412,\
        2.3009044, 1.6807970, 1.9258431, 2.3732799, 1.0147146, 1.6255013, 1.9335160, 3.2866492, 4.4683571,\
        2.8719037, 1.8299214, 1.5066573, 1.4453529, 0.4761787, 3.2652345};

        double r_p_value = 1.911e-05;

        TestStrategy::TestResult res = t_test(mean(a), stddev(a), a.size(),
            mean(b), stddev(b), b.size(),
            0.05, TestStrategy::TestSide::TwoSided, true);

        BOOST_CHECK_SMALL(res.pvalue - r_p_value, 0.001);
    }

    BOOST_AUTO_TEST_CASE( two_unequal_var_test )
    {

        //    Welch Two Sample t-test
        //
        //    data:  a and b
        //    t = 4.751, df = 44.777, p-value = 2.116e-05
        //    alternative hypothesis: true difference in means is not equal to 0
        //    95 percent confidence interval:
        //    0.6847557 1.6928295
        //    sample estimates:
        //    mean of x mean of y
        //    3.198143  2.009350


        double r_p_value = 2.116e-05;

        // a <- rnorm(25, 3, 1)
        Row<double> a = {3.290169, 3.031275, 2.701008, 3.703762, 4.633237, 2.327662, 3.050368, 2.634829, 3.358146,\
        3.350406, 2.490066, 3.500233, 5.485966, 3.566797, 1.945628, 2.878948, 2.920190, 3.402363, 2.821342, 3.640249,\
        4.717522, 3.353228, 2.334823, 1.997572, 2.817784};

        // b <- rnorm(23, 2, 1)
        Row<double> b = {1.8461070, 1.7434951, 2.5623408, 1.2993293, 1.7287880, 1.1736090, 2.8343341, 1.0222412,\
        2.3009044, 1.6807970, 1.9258431, 2.3732799, 1.0147146, 1.6255013, 1.9335160, 3.2866492, 4.4683571, 2.8719037,\
        1.8299214, 1.5066573, 1.4453529, 0.4761787, 3.2652345};

        TestStrategy::TestResult res = t_test(mean(a), stddev(a), a.size(),
            mean(b), stddev(b), b.size(),
            0.05, TestStrategy::TestSide::TwoSided, false);

        BOOST_CHECK_SMALL(res.pvalue - r_p_value, 0.001);
    }

BOOST_AUTO_TEST_SUITE_END()



//BOOST_FIXTURE_TEST_SUITE( test_strategy_class, sample_linear_experiment )
//
//    BOOST_AUTO_TEST_CASE( test_strategy_constructor )
//    {
//
//        linear_setup.v_means.fill(0);
//        linear_setup.v_nobs.fill(10);
//        experiment->setup.set_means(linear_setup.v_means);
//        experiment->setup.set_nobs(linear_setup.v_nobs);
//
//        rowvec pvalues;
//        rowvec statistics;
//
//        for (int i = 0; i < 10000; ++i)
//        {
//            experiment->generateData();
//            experiment->calculateStatistics();
//            experiment->runTest();
//
//            pvalues.insert_cols(pvalues.size(), experiment->pvalues);
//            statistics.insert_cols(statistics.size(), experiment->statistics);
//        }
//
//        BOOST_CHECK_SMALL(arma::mean(pvalues) - 0.5, 0.01);
//        BOOST_CHECK_SMALL(arma::mean(statistics) - 0.0, 0.01);
//
//        linear_setup.v_means.fill(1);
//        linear_setup.v_means.fill(1);
//        linear_setup.v_nobs.fill(10);
//        experiment->setup.set_means(linear_setup.v_means);
//        experiment->setup.set_means(linear_setup.v_means);
//        experiment->setup.set_nobs(linear_setup.v_nobs);
//
//        pvalues.clear();
//        statistics.clear();
//        for (int i = 0; i < 10000; ++i)
//        {
//            experiment->generateData();
//            experiment->calculateStatistics();
//            experiment->runTest();
//
//            pvalues.insert_cols(pvalues.size(), experiment->pvalues);
//            statistics.insert_cols(statistics.size(), experiment->statistics);
//        }
//
//        BOOST_CHECK_SMALL(arma::mean(pvalues) - 0.04, 0.01);
//        BOOST_CHECK_SMALL(arma::mean(statistics) - 3.45, 0.01);
//
//    }

BOOST_AUTO_TEST_SUITE_END()
