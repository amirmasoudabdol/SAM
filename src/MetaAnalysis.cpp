//
// Created by Amir Masoud Abdol on 2019-04-25.
//

#include <algorithm>
#include <iostream>

#include <spdlog/spdlog.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include "sam.h"

#include "MetaAnalysis.h"
#include "Journal.h"
#include <boost/math/distributions/normal.hpp>
#include <boost/math/distributions/students_t.hpp>
#include <boost/math/distributions/non_central_t.hpp>
#include <boost/math/distributions/chi_squared.hpp>

// #include <mlpack/core.hpp>
#include <mlpack/methods/linear_regression/linear_regression.hpp>

using namespace std;
using namespace sam;

MetaAnalysis::~MetaAnalysis(){

};

std::unique_ptr<MetaAnalysis> MetaAnalysis::build(std::string name) {
  if (name == "FixedEffectEstimator") {
    return std::make_unique<FixedEffectEstimator>();
  }else if (name == "RandomEffectEstimator") {
    return std::make_unique<RandomEffectEstimator>();
  }else if (name == "EggersTestEstimator") {
    return std::make_unique<EggersTestEstimator>();
  }else if (name == "TestOfObsOverExptSig") {
    return std::make_unique<TestOfObsOverExptSig>();
  }

  return nullptr;
}

std::vector<std::string> MetaAnalysis::Columns(std::string name) {
  if (name == "FixedEffectEstimator") {
    return FixedEffectEstimator::ResultType::Columns();
  }else if (name == "RandomEffectEstimator") {
    return RandomEffectEstimator::ResultType::Columns();
  }else if (name == "EggersTestEstimator") {
    return EggersTestEstimator::ResultType::Columns();
  }else if (name == "TestOfObsOverExptSig") {
    return TestOfObsOverExptSig::ResultType::Columns();
  }
  
  return {};
}

void FixedEffectEstimator::estimate(Journal *journal) {
  
  journal->meta_analysis_submissions.push_back(FixedEffect(journal->yi, journal->vi));
}

void RandomEffectEstimator::estimate(Journal *journal) {

  auto tau2_DL = RandomEffectEstimator::DL(journal->yi, journal->vi, journal->wi);
  
  journal->meta_analysis_submissions.push_back(RandomEffect(journal->yi, journal->vi, tau2_DL));
}


void EggersTestEstimator::estimate(Journal *journal) {
  
  journal->meta_analysis_submissions.push_back(EggersTest(journal->yi, journal->vi, params.alpha));
}


RandomEffectEstimator::ResultType
RandomEffectEstimator::RandomEffect(const arma::Row<double> &yi, const arma::Row<double> &vi, double tau2) {
  
  using boost::math::normal;
  using boost::math::chi_squared;
  
  normal norm(0, 1);
  
  // Weight per study
  arma::rowvec wi = 1. / (vi + tau2);
  // Meta-analytic estimate
  auto est = arma::accu(yi % wi) / arma::accu(wi);
  // Standard error of meta-analytic estimate
  auto se = sqrt(1. / arma::accu(wi));
  // Lower bound CI meta-analytical estimate
  auto ci_lb = est - quantile(norm, 0.975) * se;
  // Upper bound CI meta-analytical estimate
  auto ci_ub = est + quantile(norm, 0.975) * se;
  // Z-value for test of no effect
  auto zval = est/se;
  // Compute one-sided p-value
  auto pval_one = cdf(complement(norm, zval));
  // Compute two-tailed p-value
  auto pval = pval_one > 0.5 ? (1. - pval_one) * 2 : pval_one * 2;
    
  arma::rowvec wi_fe = 1. / vi;
  auto est_fe = arma::accu(wi_fe % yi)/ arma::accu(wi_fe);
  
  // Q-statistic
  auto q_stat = arma::accu(wi_fe % arma::pow(yi - est_fe, 2));
  
  chi_squared chisq(yi.n_elem - 1);
  // p-value of Q-statistic
  auto q_pval = cdf(complement(chisq, q_stat));
  
  return ResultType{est, se, ci_lb, ci_ub, zval, pval, q_stat, q_pval};
}


// General method-of-moments estimate (Eq. 6 in DerSimonian and Kacker, 2007)
double RandomEffectEstimator::DL(const arma::Row<double> &yi, const arma::Row<double> &vi, const arma::Row<double> &ai) {
  
  auto yw = arma::accu(ai % yi) / arma::accu(ai);
  auto est_tau2 = (arma::accu(ai % arma::pow(yi-yw, 2))-(arma::accu(ai % vi)-arma::accu(arma::pow(ai, 2) % vi)/arma::accu(ai)))/(arma::accu(ai)-arma::accu(arma::pow(ai, 2))/arma::accu(ai));
  est_tau2 = est_tau2 < 0 ? 0 : est_tau2;
  
  return (est_tau2);
}

// Function for estimating tau2 with Paule-Mandel estimator
double RandomEffectEstimator::PM(const arma::Row<double> &yi, const arma::Row<double> &vi, const double tau2) {
  // Degrees of freedom of Q-statistic (df is also expected value because chi square distributed)
  auto df = yi.n_elem - 1;
    // Weights in meta-analysis
  arma::rowvec wi = 1. / (vi + tau2);
    // Meta-analytic effect size
  auto theta = arma::accu(yi % wi)/arma::accu(wi);
    // Q-statistic
  auto Q = arma::accu(wi % arma::pow(yi - theta, 2));

  // Stop iterating if computed Q-statistic equals degrees of freedom

  return (Q - df);
}

EggersTestEstimator::ResultType
EggersTestEstimator::EggersTest(const arma::Row<double> &yi, const arma::Row<double> &vi, double alpha) {
  
  using namespace mlpack;
  using namespace mlpack::regression;
  
  using boost::math::students_t;
  
  auto n = yi.n_elem;
  auto p = 2;
  double df = n - p;
  
  arma::rowvec wi = 1./vi;
  arma::rowvec wts = arma::sqrt(wi);
  arma::rowvec si = arma::sqrt(vi);
  
  arma::rowvec predictions(n);
  
  arma::mat X(2, n);
  X.row(0) = arma::ones(n).as_row();
  X.row(1) = si;
  
  LinearRegression lg(X, yi, wi);
  lg.Train(X, yi, wi, false);
  lg.Predict(X, predictions);

  arma::rowvec errors = yi - predictions;
  
  auto slope = lg.Parameters().at(1);

  arma::mat W = arma::diagmat(wi);
  
  double res_var_2 = sqrt(arma::accu(wi % arma::pow(errors, 2)) / (n - 2));
  arma::mat S_2 = arma::diagmat(arma::pow(res_var_2 / sqrt(wi), 2));
  
  arma::mat Z = X.t();
  arma::mat var_betas = arma::sqrt(arma::inv(Z.t() * W * Z) * (Z.t() * W * S_2 * W.t() * Z) * arma::inv(Z.t() * W * Z));
  
  double slope_se = var_betas.diag().at(1);
  
  double slope_stat = slope / slope_se;
  
  auto res = TTest::compute_pvalue(slope_stat, n - 2, 0.1, TestStrategy::TestAlternative::TwoSided);
  
  return ResultType{slope, slope_se, slope_stat, res.first, res.second, df};
}

using namespace sam;

sam::TestOfObsOverExptSig::ResultType
TestOfObsOverExptSig::TES(const arma::Row<double> &sigs, const arma::Row<double> &ni, double beta, double alpha) {
  
  using boost::math::students_t;
  using boost::math::non_central_t;
  using boost::math::chi_squared;
  
  double k = sigs.n_elem;
  
  students_t tdist(k - 1);
  
  double O = arma::accu(sigs);
  
  double tcv = quantile(tdist, 0.95);
  
  arma::rowvec p_ncts(k);
  p_ncts.imbue([&, i = 0]() mutable {
    non_central_t nct(ni[i] - 1, beta * sqrt(ni[i])); i++;
    return cdf(nct, tcv);
  });

  arma::rowvec power = 1. - p_ncts;

  double E = arma::accu(power);

  double A = pow(O - E, 2) / E + pow(O - E, 2)/(k - E);

  chi_squared chisq(1);
  double pval = 1. - cdf(chisq, A);
    
  return TestOfObsOverExptSig::ResultType{E, A, pval, pval < alpha};
}


void TestOfObsOverExptSig::estimate(Journal *journal) {
  
  double beta = FixedEffectEstimator::FixedEffect(journal->yi, journal->vi).est;
  
  arma::rowvec sigs(journal->yi.n_elem);
  sigs.imbue([&, i = 0]() mutable {
    return journal->publications_list[i++].group_.sig_;
  });
  
  arma::rowvec ni(journal->yi.n_elem);
  ni.imbue([&, i = 0]() mutable {
    return journal->publications_list[i++].group_.nobs_;
  });
  
  
  journal->meta_analysis_submissions.push_back(TestOfObsOverExptSig::TES(sigs, ni, beta, 0.05));
}
