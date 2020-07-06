//
// Created by Amir Masoud Abdol on 2019-04-25.
//

#include <algorithm>
#include <iostream>

#include "MetaAnalysis.h"
#include "Journal.h"
#include <boost/math/distributions/normal.hpp>
#include <boost/math/distributions/chi_squared.hpp>

using namespace std;
using namespace sam;

MetaAnalysis::~MetaAnalysis(){

};

std::unique_ptr<MetaAnalysis> MetaAnalysis::build(std::string name) {
  if (name == "FixedEffectEstimator") {
    return std::make_unique<FixedEffectEstimator>();
  }else if (name == "RandomEffectEstimator") {
    return std::make_unique<RandomEffectEstimator>();
  }

  return nullptr;
}

void FixedEffectEstimator::estimate(Journal *journal) {
  
  auto n = journal->publications_list.size();
  arma::Row<double> yi(n);
  arma::Row<double> vi(n);
  
  for (int i{0}; i < n; ++i) {
    yi[i] = journal->publications_list[i].group_.effect_;
    vi[i] = journal->publications_list[i].group_.var_;
  }
  
  journal->meta_analysis_submissions.push_back(FixedEffect(yi, vi));
}

void RandomEffectEstimator::estimate(Journal *journal) {
  auto n = journal->publications_list.size();
  arma::Row<double> yi(n);
  arma::Row<double> vi(n);
  
  for (int i{0}; i < n; ++i) {
    yi[i] = journal->publications_list[i].group_.effect_;
    vi[i] = journal->publications_list[i].group_.var_;
  }
  
  arma::Row<double> ai = 1./vi;
  auto tau2_DL = RandomEffectEstimator::DL(yi, vi, ai);
  
  journal->meta_analysis_submissions.push_back(RandomEffect(yi, vi, tau2_DL));
}


RandomEffectEstimator::ResultType
RandomEffectEstimator::RandomEffect(const arma::Row<double> &yi, const arma::Row<double> &vi, double tau2) {
  
  using boost::math::normal;
  using boost::math::chi_squared;
  
  normal norm(0, 1);
  
  // Weight per study
  auto wi = 1. / (vi + tau2);
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
    
  auto wi_fe = 1. / vi;
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
  auto wi = 1. / (vi + tau2);
    // Meta-analytic effect size
  auto theta = arma::accu(yi % wi)/arma::accu(wi);
    // Q-statistic
  auto Q = arma::accu(wi % arma::pow(yi - theta,2));

  // Stop iterating if computed Q-statistic equals degrees of freedom

  return (Q - df);
}
