//
// Created by Amir Masoud Abdol on 2019-04-30
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE Experiment Tests

#include <boost/test/unit_test.hpp>
namespace tt = boost::test_tools;

#include <armadillo>
#include <iostream>

#include "Experiment.h"
#include "sample_experiment_setup.h"

using namespace arma;
using namespace sam;
using namespace std;


BOOST_FIXTURE_TEST_SUITE ( constructors, ExperimentSetupSampleConfigs )

BOOST_AUTO_TEST_CASE( default_constructor ) {
  
  Experiment expr;
  
  BOOST_TEST(expr.setup.nc() == 0);
  BOOST_TEST(expr.setup.nd() == 0);
  BOOST_TEST(expr.setup.ng() == 0);
  
  BOOST_TEST(expr.isHacked() == false);
  BOOST_TEST(expr.hasCandidates() == false);
  BOOST_TEST(expr.isModified() == false);
  
}


BOOST_AUTO_TEST_CASE( from_config )
{
  
  Experiment expr{sample_experiment_setup["experiment_parameters"]};
  
  BOOST_TEST(expr.dvs_.size() == expr.setup.ng());
  
  for (int i{0}; i < expr.dvs_.size(); i++) {
    BOOST_TEST(expr.dvs_[i].id_ == i);
  }
  
  // Checking the default order of DVs
  int i{0};
  for (auto &dv : expr.dvs_) {
    BOOST_TEST(dv.id_ == i++);
  }

}

BOOST_AUTO_TEST_CASE( from_expr_setup )
{
  ExperimentSetup setup{sample_experiment_setup["experiment_parameters"]};
  Experiment expr{setup};

  BOOST_TEST(expr.dvs_.size() == expr.setup.ng());
  
  for (int i{0}; i < expr.dvs_.size(); i++) {
    BOOST_TEST(expr.dvs_[i].id_ == i);
  }
  
  // Checking the default order of DVs
  int i{0};
  for (auto &dv : expr.dvs_) {
    BOOST_TEST(dv.id_ == i++);
  }
  
}

BOOST_AUTO_TEST_CASE( from_components )
{
  
//  ExperimentSetup setup{sample_experiment_setup["experiment_parameters"]};
//
//  auto ds_config = sample_experiment_setup["data_strategy"];
//  auto ds = DataStrategy::build(ds_config);
//
//  auto ts_config = sample_experiment_setup["test_strategy"];
//  auto ts = TestStrategy::build(ts_config);
//
//  auto es_config = sample_experiment_setup["effect_strategy"];
//  auto es = EffectStrategy::build(es_config);
//
//  Experiment expr(setup, *ds, *ts, *es);
//
//  BOOST_TEST(expr.setup.ng() == ng);
//
//  for (int i{0}; i < expr.dvs_.size(); i++) {
//    BOOST_TEST(expr.dvs_[i].id_ == i);
//  }

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE ( methods_tests, ExperimentSetupSampleConfigs )


BOOST_AUTO_TEST_CASE( status_methods ) {
  
  Experiment expr{sample_experiment_setup["experiment_parameters"]};
  
  BOOST_TEST(expr.isHacked() == false);
  
  expr.setHackedStatus(true);
  BOOST_TEST(expr.isHacked() == true);
  
  expr.setHackedStatus(false);
  expr.setHackedStatusOf({2}, true);
  BOOST_TEST(expr.isHacked() == true);
  BOOST_TEST(expr.isModified() == true);
  
  
  expr.setCandidateStatusOf({2, 3}, true);
  BOOST_TEST(expr.hasCandidates() == true);
  BOOST_TEST(expr.nCandidates() == 2);
  
  expr.setPublishedStatus(true);
  BOOST_TEST(expr.isPublished() == true);
  
  expr.clear();
  BOOST_TEST(expr.isHacked() == false);
  BOOST_TEST(expr.isModified() == false);
  BOOST_TEST(expr.hasCandidates() == false);
  BOOST_TEST(expr.nCandidates() == 0);
  BOOST_TEST(expr.isPublished() == false);
  
}


BOOST_AUTO_TEST_CASE( calculation_methods ) {
  
  Experiment expr{sample_experiment_setup["experiment_parameters"]};
  
  for (auto &dv : expr.dvs_) {
    BOOST_TEST(dv.nobs_ == 0);
  }
  
  expr.generateData();
  for (auto &dv : expr.dvs_) {
    BOOST_TEST(dv.nobs_ == 10);
    BOOST_TEST(dv.mean_ == 0);
  }
  
  expr.calculateStatistics();
  for (auto &dv : expr.dvs_) {
    BOOST_TEST(dv.mean_ != 0);
    BOOST_TEST(dv.var_ != 0);
  }
  
  expr.calculateTests();
  for (int i{expr.setup.nd()}; i < expr.setup.ng(); i++) {
    BOOST_TEST(expr.dvs_[i].stats_ != 0);
    BOOST_TEST(expr.dvs_[i].pvalue_ != 0);
  }
  
  expr.calculateEffects();
  for (int i{expr.setup.nd()}; i < expr.setup.ng(); i++) {
    BOOST_TEST(expr.dvs_[i].effect_ != 0);
    BOOST_TEST(expr.dvs_[i].eff_side_ != 0);
  }
  
  auto id_2_pvalue = expr.dvs_[2].pvalue_;
  expr.dvs_[2].removeMeasurements({1, 2, 3});
  BOOST_TEST(expr.dvs_[2].pvalue_ == id_2_pvalue);
  expr.recalculateEverything();
  BOOST_TEST(expr.dvs_[2].pvalue_ != id_2_pvalue);
  
}

BOOST_AUTO_TEST_SUITE_END()

//	BOOST_AUTO_TEST_CASE( linear_data_strategy_testing_stats )
//	{
//
//        // srand(42);
//
//	    auto ts = TestStrategy::build(tsp);
//
//	    auto ds = DataStrategy::build(dsp);
//
//        auto es = EffectStrategy::build(esp);
//
////        v_nobs.fill(5000);
////        setup.set_nobs(v_nobs);
//
//        Experiment expr(setup, ds, ts, es);
//
//	    expr.generateData();
//	    expr.calculateStatistics();
//
//	    BOOST_TEST(expr.measurements.size() == ng);
//
//	    BOOST_TEST(expr.means.size() == ng);
//
//        // Checking means ---------------------------------
//	    for (int i = 0; i < expr.setup.ng(); ++i)
//	        BOOST_CHECK_SMALL(expr.means[i] - v_means[i], 0.1);
//
//        // Checking variance ------------------------------
//        for (int i = 0; i < expr.setup.ng(); ++i)
//            BOOST_CHECK_SMALL(expr.vars[i] - v_vars[i], 0.1);
//
//        // Checking covariance ----------------------------
//        arma::Mat<float> dt(nobs, ng);
//        for (int i = 0; i < expr.setup.ng(); ++i)
//            dt.col(i) = expr.measurements[i].as_col();
//
//        arma::Mat<float> corr = arma::cor(dt);
//        for (int i = 0; i < setup.ng(); ++i)
//            for (int j = 0; j < setup.ng(); ++j)
//                BOOST_CHECK_SMALL(corr(i, j) - v_sigma(i, j), 0.1);
//
//	}



