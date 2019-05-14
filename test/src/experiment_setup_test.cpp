//
// Created by Amir Masoud Abdol on 2019-04-30
//


#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE ExperimentSetup Tests

#include <boost/test/unit_test.hpp>
namespace tt = boost::test_tools;

#include <armadillo>
#include <iostream>
#include <vector>
#include <algorithm>


#include "sam.h"
#include "Experiment.h"
#include "ExperimentSetup.h"

using namespace arma;
using namespace sam;
using namespace std;

BOOST_AUTO_TEST_SUITE( constructors )

    BOOST_AUTO_TEST_CASE( default_explicit_constructor )
    {
        ExperimentSetup setup;

        BOOST_TEST(setup.ng() == 0);
        BOOST_TEST(setup.ni() == 0);
        BOOST_TEST(accu(setup.means()) == 0);
    }

    BOOST_AUTO_TEST_CASE ( linear_setup_constructor )
    {

        TestStrategy::TestStrategyParameters test_params;
        test_params.name = TestStrategy::TestType::TTest;
        test_params.side = TestStrategy::TestSide::TwoSide;
        test_params.alpha = 0.05;

        DataStrategyParameters data_params;
        data_params.name = ExperimentType::LinearModel;

        ExperimentSetup setup(2, 3, 
                              20, .147, 1.0, 0.1,
                              test_params, data_params);

        BOOST_TEST(setup.ng() == 6);
        BOOST_TEST(setup.ni() == 0);
        BOOST_TEST(setup.nrows() == 0);

        BOOST_TEST(setup.means() == mat(1, 6).fill(0.147),
                     tt::per_element());

        BOOST_TEST(setup.vars() == mat(1, 6).fill(1.0),
                     tt::per_element());

        vec sdiag = setup.sigma().diag();
        BOOST_TEST(sdiag == vec(6).fill(1.0),
                    tt::per_element());

        sdiag = setup.sigma().diag(1);
        BOOST_TEST(sdiag == vec(5).fill(0.1),
                    tt::per_element());
    }

    BOOST_AUTO_TEST_CASE( linear_setup_parameter_modification )
    {

        int nc = 2;
        int nd = 3;
        int ng = nc * nd;

        TestStrategy::TestStrategyParameters test_params;
        test_params.name = TestStrategy::TestType::TTest;
        test_params.side = TestStrategy::TestSide::TwoSide;
        test_params.alpha = 0.05;

        DataStrategyParameters data_params;
        data_params.name = ExperimentType::LinearModel;

        ExperimentSetup setup(nc, nd, 
                              20, .147, 1.0, 0.1,
                              test_params, data_params);

        rowvec v = rowvec(ng);

        v = rowvec(ng).fill(1);
        setup.set_means(v);
        BOOST_TEST(setup.means() == mat(1, 6).fill(1),
                     tt::per_element());

        v = rowvec(ng).fill(1);
        setup.set_vars(v);
        BOOST_TEST(setup.vars() == mat(1, 6).fill(1),
                     tt::per_element());


        vec vars(ng); vars.fill(5.0);
        mat sigma(ng, ng);
        sigma.fill(0.01);
        sigma.diag() = vars;


        // Checking if the changes reflect in the object
        setup.set_sigma(sigma);
        
        vec dg = setup.sigma().diag();
        BOOST_TEST(dg == vec(6).fill(5.0),
                        tt::per_element());

        vec udg = setup.sigma().diag(1);
        BOOST_TEST(udg  == vec(5).fill(0.01),
                    tt::per_element());

    }


    BOOST_AUTO_TEST_CASE( linear_setup_matrix_constructor_fixed_covs )
    {

        int nc = 2;
        int nd = 5;
        int ng = nc * nd;
        arma::Row<int> nobs(ng);
        std::generate(nobs.begin(), nobs.end(), [i=0]() mutable {return i++;});
        rowvec means = linspace<rowvec>(0, 100, 10);
        rowvec vars = linspace<rowvec>(0, 1, 10);
        double covs = 0.0001;

        mat sigma(ng, ng);
        sigma.fill(covs);
        sigma.diag() = vars;

        TestStrategy::TestStrategyParameters test_params;
        test_params.name = TestStrategy::TestType::TTest;
        test_params.side = TestStrategy::TestSide::TwoSide;
        test_params.alpha = 0.05;

        DataStrategyParameters data_params;
        data_params.name = ExperimentType::LinearModel;

        ExperimentSetup setup(nc, nd, 
                              nobs, means, vars, covs,
                              test_params, data_params);

        BOOST_TEST(setup.ng() = ng);

        BOOST_TEST(setup.nobs() == nobs,
                     tt::per_element());

        BOOST_TEST(setup.means() == means,
                     tt::per_element());

        BOOST_TEST(setup.sigma() == sigma,
                     tt::per_element());

    }

    BOOST_AUTO_TEST_CASE( linear_setup_matrix_constructor_sigma_mat )
    {

        int nc = 2;
        int nd = 5;
        int ng = nc * nd;
        arma::Row<int> nobs(ng);
        std::generate(nobs.begin(), nobs.end(), [i=0]() mutable {return i++;});
        rowvec means = linspace<rowvec>(0, 100, 10);
        rowvec vars = linspace<rowvec>(0, 1, 10);
        mat sigma(ng, ng); sigma.randn();

        TestStrategy::TestStrategyParameters test_params;
        test_params.name = TestStrategy::TestType::TTest;
        test_params.side = TestStrategy::TestSide::TwoSide;
        test_params.alpha = 0.05;

        DataStrategyParameters data_params;
        data_params.name = ExperimentType::LinearModel;

        ExperimentSetup setup(nc, nd, 
                              nobs, means, sigma,
                              test_params, data_params);

        BOOST_TEST(setup.ng() = ng);

        BOOST_TEST(setup.nobs() == nobs,
                     tt::per_element());

        BOOST_TEST(setup.means() == means,
                     tt::per_element());

        BOOST_TEST(setup.sigma() == sigma,
                     tt::per_element());

    }

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( experiment_setup_builder )

    BOOST_AUTO_TEST_CASE( building_with_fixed_params )
    {

        int nc = 2;
        int nd = 3;
        int ng = nc * nd;

        int nobs = 25;
        double mean = 0.25;
        double var = 1;
        double cov = 0.01;

        ExperimentSetup setup = ExperimentSetup::create().setNumConditions(nc)
                                .setNumDependentVariables(nd)
                                .setNumItems(0)
                                .setFixedNumObservations(nobs)
                                .setFixedMeans(mean)
                                .setFixedVariance(var)
                                .setFixedCovariance(cov);

        arma::Row<int> v_nobs = arma::Row<int>(ng).fill(nobs);
        BOOST_TEST( setup.nobs() == v_nobs,
                    tt::per_element());

        arma::Row<double> v_means = arma::Row<double>(ng).fill(mean);
        BOOST_TEST( setup.means() == v_means,
                        tt::per_element());

        arma::Row<double> v_vars = arma::Row<double>(ng).fill(var);
        BOOST_TEST( setup.vars() == v_vars,
                    tt::per_element());

        arma::Mat<double> v_sigma;
        v_sigma = arma::Mat<double>(ng, ng).fill(cov);
        v_sigma.diag() = v_vars;
        BOOST_TEST( setup.sigma() == v_sigma,
                        tt::per_element());

    }

    BOOST_AUTO_TEST_CASE( building_with_arrays )
    {

    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( parameter_randomization )

    BOOST_AUTO_TEST_CASE( random_n_obs )
    {
        // int nc = 2;
        // int nd = 3;
        // int ng = nc * nd;

        // int nobs = 0;
        // double mean = 0.1;
        // double var = 1;
        // double cov = 0; 

        // ExperimentSetup setup(nc, nd, 
        //                       nobs, mean, var, cov);

        // setup.setSeed(42);

        // arma::rowvec nobs_v = {41.6262, 35.6870, 43.8443, 48.0483, 23.2773, 49.5977};

        // setup.set_nobs(); // ", 20, 50);)

        // for (int i = 0; i < nobs_v.size(); ++i)
        // {
        //     BOOST_CHECK_SMALL(nobs_v[i] - setup.nobs()[i], 0.0001);
        // }

    }

BOOST_AUTO_TEST_SUITE_END()




