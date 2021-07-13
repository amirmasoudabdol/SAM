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

        ///    t.test(a)
        ///    -----------------
        ///    One Sample t-test
        ///
        ///    data:  a
        ///    t = 19.287, df = 24, p-value = 4.077e-16
        ///    alternative hypothesis: true mean is not equal to 0
        ///    95 percent confidence interval:
        ///     2.855905 3.540381
        ///    sample estimates:
        ///    mean of x
        ///     3.198143

        // a <- rnorm(25, 3, 1)
        Row<float> a = {3.290169, 3.031275, 2.701008, 3.703762, 4.633237, 2.327662, 3.050368,\
        2.634829, 3.358146, 3.350406, 2.490066, 3.500233, 5.485966, 3.566797, 1.945628, 2.878948,\
        2.920190, 3.402363, 2.821342, 3.640249, 4.717522, 3.353228, 2.334823, 1.997572, 2.817784};

        float r_p_value = 4.077e-16;

        TestStrategy::TestResult res = t_test(0., 0., 0.,
                mean(a), stddev(a), a.size(),
                0.05, TestStrategy::TestSide::TwoSided, true);

        BOOST_CHECK_SMALL(res.pvalue - r_p_value, 0.001);
    }

    BOOST_AUTO_TEST_CASE( two_equal_var_test )
    {

        ///    t.test(a, b, equal.var = TRUE)
        ///    ------------------------------
        ///     Welch Two Sample t-test
        ///
        ///     data:  a and b
        ///     t = 4.751, df = 44.777, p-value = 2.116e-05
        ///     alternative hypothesis: true difference in means is not equal to 0
        ///     95 percent confidence interval:
        ///      0.6847557 1.6928296
        ///     sample estimates:
        ///     mean of x mean of y
        ///      3.198143  2.009350

        // a <- rnorm(25, 3, 1)
        Row<float> a = {3.290169, 3.031275, 2.701008, 3.703762, 4.633237, 2.327662, 3.050368, 2.634829,\
        3.358146, 3.350406, 2.490066, 3.500233, 5.485966, 3.566797, 1.945628, 2.878948, 2.920190, 3.402363,\
        2.821342, 3.640249, 4.717522, 3.353228, 2.334823, 1.997572, 2.817784};

        // b <- rnorm(23, 2, 1)
        Row<float> b = {1.8461070, 1.7434951, 2.5623408, 1.2993293, 1.7287880, 1.1736090, 2.8343341, 1.0222412,\
        2.3009044, 1.6807970, 1.9258431, 2.3732799, 1.0147146, 1.6255013, 1.9335160, 3.2866492, 4.4683571,\
        2.8719037, 1.8299214, 1.5066573, 1.4453529, 0.4761787, 3.2652345};

        float r_p_value = 2.116e-05;

        TestStrategy::TestResult res = t_test(mean(a), stddev(a), a.size(),
            mean(b), stddev(b), b.size(),
            0.05, TestStrategy::TestSide::TwoSided, true);

        BOOST_CHECK_SMALL(res.pvalue - r_p_value, 0.001);
    }

    BOOST_AUTO_TEST_CASE( two_unequal_var_test )
    {

        ///    t.test(a, b, equal.var = TRUE)
        ///    ------------------------------
        ///     Welch Two Sample t-test
        ///
        ///     data:  a and b
        ///     t = 2.1232, df = 47.793, p-value = 0.03894
        ///     alternative hypothesis: true difference in means is not equal to 0
        ///     95 percent confidence interval:
        ///      0.03276729 1.20544485
        ///     sample estimates:
        ///     mean of x mean of y
        ///      2.748049  2.128943


        float r_p_value = 0.03894;

        // a <- rnorm(25, 3, 1)
        arma::Row<float> a = {2.458769, 3.477696, 3.463705, 1.861244, 1.559779, 2.277873, 3.653672, 2.671046, 1.720600, 1.47238, 3.074170, 3.349859, 3.116968, 1.728138, 5.466796, 4.090923, 3.094437, 2.798243, 2.720764, 3.69761, 1.969404, 1.020822, 1.725920, 3.137324, 3.093084};

        // b <- rnorm(23, 2, 1.25)
        arma::Row<float> b = {3.3114564, 2.1292719, 1.0175327, 0.5345535, 4.5111627, 1.4445051, 3.7719721, 1.9244836, 1.924285, 0.7262634, 2.1259286, 3.9777595, 2.5783889, 1.3391029, 2.1343111, 2.4064448, 2.0156767, 2.830056, 1.3926431, 2.6001390, 3.0546628, 2.5279317, 1.0209667, 0.4325975, 1.4914873};

        TestStrategy::TestResult res = t_test(mean(a), stddev(a), a.size(),
            mean(b), stddev(b), b.size(),
            0.05, TestStrategy::TestSide::TwoSided, false);
        
        BOOST_CHECK_SMALL(res.pvalue - r_p_value, 0.001);
    }

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE ( yuen_t_test, SampleResearch )
    
    BOOST_AUTO_TEST_CASE( yuen_one_sample )
    {
        
        ///     One sample Yuen test, trim=0.2
        ///
        ///     data:  x
        ///     t = 24.479, df = 14, p-value = 6.839e-13
        ///     alternative hypothesis: true trimmed means is not equal to 0
        ///     95 percent confidence interval:
        ///      2.839161 3.384451
        ///     sample estimates:
        ///     trimmed mean of x
        ///              3.111806
        
        // a <- rnorm(25, 3, 1)
        arma::Row<float> a = {3.290169, 3.031275, 2.701008, 3.703762, 4.633237, 2.327662, 3.050368,\
        2.634829, 3.358146, 3.350406, 2.490066, 3.500233, 5.485966, 3.566797, 1.945628, 2.878948,\
        2.920190, 3.402363, 2.821342, 3.640249, 4.717522, 3.353228, 2.334823, 1.997572, 2.817784};
        
        float r_p_value {6.839e-13};
        
        auto res = yuen_t_test_one_sample(a, 0.05, TestStrategy::TestSide::TwoSided, 0.2, 0.0);
        
        BOOST_CHECK_SMALL(res.pvalue - r_p_value, 0.0001);
        
    }
    
    BOOST_AUTO_TEST_CASE( yuen_two_samples )
    {
     
        ///     Two-sample Yuen test, trim=0.2
        ///
        ///     data:  x and y
        ///     t = 5.2702, df = 24.76, p-value = 1.91e-05
        ///     alternative hypothesis: true difference in trimmed means is not equal to 0
        ///     95 percent confidence interval:
        ///      0.7324855 1.6729703
        ///     sample estimates:
        ///     trimmed mean of x trimmed mean of y
        ///              3.111806          1.909078
        
        // a <- rnorm(25, 3, 1)
        arma::Row<float> a = {3.290169, 3.031275, 2.701008, 3.703762, 4.633237, 2.327662, 3.050368, 2.634829, 3.358146, 3.350406, 2.490066, 3.500233, 5.485966, 3.566797, 1.945628, 2.878948, 2.920190, 3.402363, 2.821342, 3.640249, 4.717522, 3.353228, 2.334823, 1.997572, 2.817784};

        // b <- rnorm(23, 2, 1)
        arma::Row<float> b = {1.8461070, 1.7434951, 2.5623408, 1.2993293, 1.7287880, 1.1736090, 2.8343341, 1.0222412, 2.3009044, 1.6807970, 1.9258431, 2.3732799, 1.0147146, 1.6255013, 1.9335160, 3.2866492, 4.4683571, 2.8719037, 1.8299214, 1.5066573, 1.4453529, 0.4761787, 3.2652345};
        
        float r_p_value {1.91e-05};
        
        auto res = yuen_t_test_two_samples(a, b, 0.05, TestStrategy::TestSide::TwoSided, 0.2, 0.0);
        
        BOOST_CHECK_SMALL(res.pvalue - r_p_value, 0.0001);
        
    }
    
    BOOST_AUTO_TEST_CASE( yuen_two_paired )
    {
        
        ///      Paired Yuen test, trim=0.2
        ///
        ///      data:  x and y
        ///      t = 1.4826, df = 14, p-value = 0.1603
        ///      alternative hypothesis: true difference in trimmed means is not equal to 0
        ///      95 percent confidence interval:
        ///       -0.1849025  1.0129415
        ///      sample estimates:
        ///      trimmed mean of x - trimmed mean of y
        ///                                  0.4140195
        
        
        arma::Row<float> a = {0.822387266,-0.064217638,-1.181746398, 1.138105728, 0.101791859, 0.364959551, 1.050402437, 1.526221058,-0.112344447, 0.299409268,-1.324905954, 0.073325737, 1.146891955, 0.213061056,-0.593373924,-2.226816464, 0.851665745,-0.006791175,-0.952173744, 0.206451596, 0.285228511,-2.153039339, 0.493613284, 0.019334722,-0.288007564};
        
        arma::Row<float> b = {-0.89960154,-0.25609764, 0.44374836,-0.94571778, -0.89574987,  1.37989272,-1.69240306, -0.32672914,-0.97609125, 0.69973678,-0.54648018, -0.27665633, -0.16372239,-0.59180641, 0.46706682, 0.08999496, 0.08881282,-1.04699859, -1.70451863, -0.18635738, 0.55473149, 1.06225983, 1.70725958,-0.39616876,-1.63021445};
        
        float r_p_value {0.1603};
        
        auto res = yuen_t_test_paired(a, b, 0.05, TestStrategy::TestSide::TwoSided, 0.2, 0.0);
        
        BOOST_CHECK_SMALL(res.pvalue - r_p_value, 0.0001);

    }
    
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE ( wilcox_test , SampleResearch )

    BOOST_AUTO_TEST_CASE( wilcox_two_sample ) {

        ///        > wilcox.test(a, b)
        ///
        ///            Wilcoxon rank sum test
        ///
        ///        data:  a and b
        ///        W = 375, p-value = 0.2312
        ///        alternative hypothesis: true location shift is not equal to 0
         
         
         arma::Row<float> a = {0.822387266,-0.064217638,-1.181746398, 1.138105728, 0.101791859, 0.364959551, 1.050402437, 1.526221058,-0.112344447, 0.299409268,-1.324905954, 0.073325737, 1.146891955, 0.213061056,-0.593373924,-2.226816464, 0.851665745,-0.006791175,-0.952173744, 0.206451596, 0.285228511,-2.153039339, 0.493613284, 0.019334722,-0.288007564};
         
         arma::Row<float> b = {-0.89960154,-0.25609764, 0.44374836,-0.94571778, -0.89574987,  1.37989272,-1.69240306, -0.32672914,-0.97609125, 0.69973678,-0.54648018, -0.27665633, -0.16372239,-0.59180641, 0.46706682, 0.08999496, 0.08881282,-1.04699859, -1.70451863, -0.18635738, 0.55473149, 1.06225983, 1.70725958,-0.39616876,-1.63021445};
         
         float r_p_value {0.2312};
         
         auto res = wilcoxon_test(a, b, 0.05, 1, TestStrategy::TestSide::TwoSided);
                 
         BOOST_CHECK_SMALL(res.pvalue - r_p_value, 0.01);
    }

BOOST_AUTO_TEST_SUITE_END()

//BOOST_FIXTURE_TEST_SUITE( test_strategy_class, SampleResearch )
//
//    BOOST_AUTO_TEST_CASE( test_strategy_constructor )
//    {
//
//        initResearch();
//
//        v_means.fill(0);
//        v_nobs.fill(10);
////        experiment->setup.set_means(v_means);
////        experiment->setup.set_nobs(v_nobs);
//
//        rowvec pvalues;
//        rowvec statistics;
//
//        for (int i = 0; i < 10000; ++i)
//        {
//            experiment->generateData();
//            experiment->calculateStatistics();
//            experiment->calculateTests();
//
//            pvalues.insert_cols(pvalues.size(), experiment->pvalues);
//            statistics.insert_cols(statistics.size(), experiment->statistics);
//        }
//
//        BOOST_CHECK_SMALL(arma::mean(pvalues) - 0.5, 0.01);
//        BOOST_CHECK_SMALL(arma::mean(statistics) - 0.0, 0.01);
//
//        v_means.fill(1);
//        v_means.fill(1);
//        v_nobs.fill(10);
////        experiment->setup.set_means(v_means);
////        experiment->setup.set_means(v_means);
//        experiment->setup.set_nobs(v_nobs);
//
//        pvalues.clear();
//        statistics.clear();
//        for (int i = 0; i < 10000; ++i)
//        {
//            experiment->generateData();
//            experiment->calculateStatistics();
//            experiment->calculateTests();
//
//            pvalues.insert_cols(pvalues.size(), experiment->pvalues);
//            statistics.insert_cols(statistics.size(), experiment->statistics);
//        }
//
//        BOOST_CHECK_SMALL(arma::mean(pvalues) - 0.04, 0.01);
//        BOOST_CHECK_SMALL(arma::mean(statistics) - 3.45, 0.01);
//
//    }
//
//BOOST_AUTO_TEST_SUITE_END()
