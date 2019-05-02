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
#include "ExperimentSetup.h"

#include "sam.h"

using namespace arma;
using namespace sam;
using namespace std;

BOOST_AUTO_TEST_SUITE( constructors )

    BOOST_AUTO_TEST_CASE( default_explicit_constructor)
    {
        ExperimentSetup setup;

        BOOST_TEST(setup.ng() == 3);
        BOOST_TEST(setup.ni() == 0);
        BOOST_TEST(accu(setup.getValueOf("means")) == 0);
    }

    BOOST_AUTO_TEST_CASE( experiment_size_constructor )
    {
        ExperimentSetup setup(3, 5);

        BOOST_TEST(setup.ng() == 15);
        BOOST_TEST(setup.ni() == 0);
        BOOST_TEST(setup.nrows() == 0);

        BOOST_TEST(accu(setup.getValueOf("means")) == 0);

        BOOST_TEST(setup.getValueOf("error_means").size() == 0);
    }

    BOOST_AUTO_TEST_CASE ( linear_setup_constructor )
    {
        ExperimentSetup setup(2, 3, 
                              20, .147, 1.0, 0.1);

        BOOST_TEST(setup.ng() == 6);
        BOOST_TEST(setup.ni() == 0);
        BOOST_TEST(setup.nrows() == 0);

        BOOST_TEST(setup.getValueOf("means") == mat(1, 6).fill(0.147),
                     tt::per_element());

        BOOST_TEST(setup.getValueOf("vars") == mat(1, 6).fill(1.0),
                     tt::per_element());

        vec sdiag = setup.getValueOf("sigma").diag();
        BOOST_TEST(sdiag == vec(6).fill(1.0),
                    tt::per_element());

        sdiag = setup.getValueOf("sigma").diag(1);
        BOOST_TEST(sdiag == vec(5).fill(0.1),
                    tt::per_element());
    }

    BOOST_AUTO_TEST_CASE( linear_setup_parameter_modification )
    {

        int nc = 2;
        int nd = 3;
        int ng = nc * nd;

        ExperimentSetup setup(nc, nd, 
                              20, .147, 1.0, 0.1);

        setup.setValueOf("nobs", 25);
        BOOST_TEST(setup.getValueOf("nobs") == mat(1, 6).fill(25),
                     tt::per_element());

        setup.setValueOf("means", 1);
        BOOST_TEST(setup.getValueOf("means") == mat(1, 6).fill(1),
                     tt::per_element());

        setup.setValueOf("vars", 1);
        BOOST_TEST(setup.getValueOf("vars") == mat(1, 6).fill(1),
                     tt::per_element());


        vec vars(ng); vars.fill(5.0);
        mat sigma(ng, ng);
        sigma.fill(0.01);
        sigma.diag() = vars;


        // Checking if the changes reflect in the object
        setup.setValueOf("sigma", sigma);
        
        vec dg = setup.getValueOf("sigma").diag();
        BOOST_TEST(dg == vec(6).fill(5.0),
                        tt::per_element());

        vec udg = setup.getValueOf("sigma").diag(1);
        BOOST_TEST(udg  == vec(5).fill(0.01),
                    tt::per_element());

    }


    BOOST_AUTO_TEST_CASE( linear_setup_matrix_constructor_fixed_covs )
    {

        int nc = 2;
        int nd = 5;
        int ng = nc * nd;
        rowvec nobs = linspace<rowvec>(0, 100, 10);
        rowvec means = linspace<rowvec>(0, 100, 10);
        rowvec vars = linspace<rowvec>(0, 1, 10);
        double covs = 0.0001;

        mat sigma(ng, ng);
        sigma.fill(covs);
        sigma.diag() = vars;

        ExperimentSetup setup(nc, nd, 
                              nobs, means, vars, covs);

        BOOST_TEST(setup.ng() = ng);

        BOOST_TEST(setup.getValueOf("nobs") == nobs,
                     tt::per_element());

        BOOST_TEST(setup.getValueOf("means") == means,
                     tt::per_element());

        BOOST_TEST(setup.getValueOf("sigma") == sigma,
                     tt::per_element());

    }

    BOOST_AUTO_TEST_CASE( linear_setup_matrix_constructor_sigma_mat )
    {

        int nc = 2;
        int nd = 5;
        int ng = nc * nd;
        rowvec nobs = linspace<rowvec>(0, 100, 10);
        rowvec means = linspace<rowvec>(0, 100, 10);
        rowvec vars = linspace<rowvec>(0, 1, 10);
        mat sigma(ng, ng); sigma.randn();

        ExperimentSetup setup(nc, nd, 
                              nobs, means, sigma);

        BOOST_TEST(setup.ng() = ng);

        BOOST_TEST(setup.getValueOf("nobs") == nobs,
                     tt::per_element());

        BOOST_TEST(setup.getValueOf("means") == means,
                     tt::per_element());

        BOOST_TEST(setup.getValueOf("sigma") == sigma,
                     tt::per_element());

    }

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( parameter_randomization )

    BOOST_AUTO_TEST_CASE( random_n_obs )
    {
        int nc = 2;
        int nd = 3;
        int ng = nc * nd;

        int nobs = 0;
        double mean = 0.1;
        double var = 1;
        double cov = 0; 

        ExperimentSetup setup(nc, nd, 
                              nobs, mean, var, cov);

        setup.setSeed(42);

        arma::rowvec nobs_v = {41.6262, 35.6870, 43.8443, 48.0483, 23.2773, 49.5977};

        setup.setValueOf("nobs", 20, 50);

        for (int i = 0; i < nobs_v.size(); ++i)
        {
            BOOST_CHECK_SMALL(nobs_v[i] - setup.getValueOf("nobs")[i], 0.0001);
        }

    }

BOOST_AUTO_TEST_SUITE_END()




