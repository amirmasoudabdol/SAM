//
// Created by Amir Masoud Abdol on 2019-05-20.
//

#ifndef SAMPP_TEST_FIXTURES_H
#define SAMPP_TEST_FIXTURES_H

#include "Experiment.h"
#include "ExperimentSetup.h"
#include "DataStrategy.h"
#include "TestStrategy.h"
#include "EffectStrategy.h"

#include "sam.h"

using namespace arma;
using namespace sam;
using namespace std;

struct sample_fixed_size_linear_experiment_setup {
    int nc = 2;
    int nd = 3;
    int ni = 0;
    int ng = nc * nd;

    int nobs = 5000;
    double mean = 0.25;
    double var = 1;
    double cov = 0.00;

    arma::Row<int> v_nobs;
    arma::Row<double> v_means;
    arma::Row<double> v_vars;
    arma::Mat<double> v_sigma;

    DataStrategy::DataStrategyParameters dsp;

    TestStrategy::TestStrategyParameters tsp;

    EffectStrategy::EffectStrategyParameters esp;

    // string ds_name = "LinearModel";

    ExperimentSetup setup;

    sample_fixed_size_linear_experiment_setup() {

        BOOST_TEST_MESSAGE( "setup the test experiment" );

        v_nobs = arma::Row<int>(ng).fill(nobs);
        v_means = arma::Row<double>(ng).fill(mean);
        v_vars = arma::Row<double>(ng).fill(var);
        v_sigma = arma::Mat<double>(ng, ng).fill(cov);
        v_sigma.diag() = v_vars;

        dsp.name = DataStrategy::DataModel::LinearModel;
        dsp.seed1 = 42;
        dsp.seed2 = 7;

        tsp.name = TestStrategy::TestType::TTest;
        tsp.side = TestStrategy::TestSide::TwoSide;
        tsp.alpha = 0.05;

        esp.name = EffectStrategy::EffectEstimator::CohensD;

        setup = ExperimentSetup::create()
                .setNumConditions(nc)
                .setNumDependentVariables(nd)
                .setNumItems(ni)
                .setNumObservations(nobs)
                .setMeans(mean)
                .setVariance(var)
                .setCovariance(cov)
                .setDataStrategy(dsp)
                .setTestStrategy(tsp)
                .setEffectStrategy(esp)
                .build();

    }
};


struct sample_linear_experiment {
    sample_fixed_size_linear_experiment_setup linear_setup;

    // ExperimentSetup setup;
    Experiment *experiment;

    sample_linear_experiment () {
        // setup = linear_setup.setup;
        experiment = new Experiment(linear_setup.setup);

    }
};

#endif //SAMPP_TEST_FIXTURES_H
