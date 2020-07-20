//
// Created by Amir Masoud Abdol on 2019-04-25.
//

#include <algorithm>
#include <iostream>
#include <cmath>

#include <spdlog/spdlog.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include "sam.h"

#include "MetaAnalysis.h"
#include "Journal.h"
#include "TestStrategy.h"
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
  }else if (name == "TrimAndFill") {
    return std::make_unique<TrimAndFill>();
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
  }else if (name == "TrimAndFill") {
    return TrimAndFill::ResultType::Columns();
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

void TrimAndFill::estimate(Journal *journal) {
  
  arma::rowvec ni(journal->yi.n_elem);
  ni.imbue([&, i = 0]() mutable {
    return journal->publications_list[i++].group_.nobs_;
  });
  
  journal->meta_analysis_submissions.push_back(TrimAndFill::TF(journal->yi, journal->vi, ni));
}

sam::TrimAndFill::ResultType TrimAndFill::TF(arma::Row<double> yi, arma::Row<double> vi, arma::Row<double> ni) {
  
  arma::rowvec wi = 1. / vi;
  
  // determine side (if none is specified)
  double beta = FixedEffectEstimator::FixedEffect(yi, vi).est;
  
  std::string side{"right"};
  std::string estimator{"R0"};
  //  if (is.null(side)) {
  //    auto res = suppressWarnings(rma.uni(yi, vi, weights=wi, mods=sqrt(vi), method=x$method, weighted=x$weighted, ...));
  // TODO: add check in case there are problems with fitting the model
  if (beta < 0) {
    side = "right";
  } else {
    side = "left";
  }
  //  }
  //else {
  //    auto side = match.arg(side, c("left", "right"));
  //  }
  
  // flip data if examining right side
  side = "right";
  if (side.find("right") != std::string::npos){
    yi = -1. * yi;
  }
  
  
  // sort data by increasing yi
  
  arma::uvec ix = arma::sort_index(yi);
  arma::rowvec yi_s = yi.elem(ix).as_row();
  arma::rowvec vi_s = vi.elem(ix).as_row();
  arma::rowvec wi_s = wi.elem(ix).as_row();
  arma::rowvec ni_s = wi.elem(ix).as_row();
  
  
  int k = yi.n_elem;
  
  
  
  int iter{0};
  int maxiter{100};
  
  double k0_sav = -1;
  double k0    =  0; // estimated number of missing studies;
  double se_k0 = 0;
  double Sr{0};
  double varSr{0};
  double k0_pval;
  
  arma::rowvec yi_c;
  arma::rowvec yi_c_r;
  arma::rowvec yi_c_r_s;
  
  while (abs(k0 - k0_sav) > 0) {
    
    k0_sav = k0; // save current value of k0;
    
    iter++;
    
    if (iter > maxiter)
      break;
    
    //  truncated data
    arma::uvec elems = arma::regspace<arma::uvec>(0, 1, k - k0 - 1);
    arma::rowvec yi_t = yi_s.elem(elems).as_row();
    arma::rowvec vi_t = vi_s.elem(elems).as_row();
    arma::rowvec wi_t = wi_s.elem(elems).as_row();
    arma::rowvec ni_t = wi_s.elem(elems).as_row();
    
    //  intercept estimate based on truncated data
    beta = FixedEffectEstimator::FixedEffect(yi_t, vi_t).est;
        
    yi_c     = yi_s - beta;                            //  centered values;
    /// \todo use the first to rank the data
    yi_c_r   = rankdata(abs(yi_c), "average").as_row(); //, ties_method="first"); //  ranked absolute centered values;
    yi_c_r_s = arma::sign(yi_c) % yi_c_r;                  //  signed ranked centered values;
    
    //  estimate the number of missing studies with the R0 estimator
    
    if (estimator.find("R0") != std::string::npos) {
      arma::uvec inx = arma::find(yi_c_r_s < 0);
      k0 = (k - arma::max(-1. * yi_c_r_s.elem(inx))) - 1;
      se_k0 = sqrt(2 * std::max(0., k0) + 2);
    }
    
    ///  estimate the number of missing studies with the L0 estimator
    if (estimator.find("L0") != std::string::npos) {
      arma::uvec inx = arma::find(yi_c_r_s > 0);
      Sr = arma::accu(yi_c_r_s.elem(inx));
      k0 = (4.*Sr - k*(k+1.)) / (2.*k - 1.);
      varSr = 1./24 * (k*(k+1.)*(2.*k+1.) + 10.*pow(k0,3) + 27.*pow(k0,2) + 17.*k0 - 18.*k*pow(k0,2) - 18.*k*k0 + 6.*pow(k,2)*k0);
      se_k0 = 4.*sqrt(varSr) / (2*k - 1);
    }
    
    ///  estimate the number of missing studies with the Q0 estimator
    if (estimator.find("Q0") != std::string::npos) {
      arma::uvec inx = arma::find(yi_c_r_s > 0);
      Sr = arma::accu(yi_c_r_s.elem(inx));
      k0 = k - 1./2 - sqrt(2*pow(k,2) - 4.*Sr + 1./4);
      varSr = 1./24 * (k*(k+1.)*(2*k+1.) + 10.*pow(k0,3) + 27.*pow(k0,2) + 17.*k0 - 18.*k*pow(k0,2) - 18.*k*k0 + 6.*pow(k,2)*k0);
      se_k0 = 2. * sqrt(varSr) / sqrt(pow(k-0.5,2) - k0*(2.*k - k0 - 1.));
    }
    
    ///  round k0 and make sure that k0 is non-negative
    k0 = std::max(0., std::round(k0));
    se_k0 = std::max(0., se_k0);
    
  }
  
  
  
  /// ------------------ Filling and estimating ----------------
  
  
  double vi_c{0};
  double imputed_beta;
  double imputed_pval;
  
  /// if estimated number of missing studies is > 0
  if (k0 > 0) {
    
    /// flip data back if side is right
    if (side.find("right") != std::string::npos) {
      yi_c = -1 * (yi_c - beta);
      yi = -1 * yi;
    } else {
      yi_c = yi_c - beta;
    }
    
    /// create filled-in data set
    arma::rowvec yi_f = yi_c;
    arma::rowvec yi_fill = yi;
    yi_fill.insert_cols(yi_f.n_elem, -1. * yi_c.elem(arma::regspace<arma::uvec>(k - k0, 1, k - 1)).as_row());
    
    /// apply limits if specified
    /// \todo: to be implemented
    //    if (!missing(ilim)) {
    //      ilim = sort(ilim)
    //      if (length(ilim) != 2L)
    //        stop(mstyle$stop("Argument 'ilim' must be of length 2_"))
    //        yi_fill[yi_fill < ilim[1]] = ilim[1]
    //        yi_fill[yi_fill > ilim[2]] = ilim[2]
    //        }
    
    arma::rowvec vi_fill = vi;
    vi_fill.insert_cols(vi.n_elem, vi.elem(arma::regspace<arma::uvec>(k - k0, 1, k - 1)).as_row());
    arma::rowvec wi_fill = wi;
    wi_fill.insert_cols(wi.n_elem, wi.elem(arma::regspace<arma::uvec>(k - k0, 1, k - 1)).as_row());
    arma::rowvec ni_fill = ni;
    ni_fill.insert_cols(ni.n_elem, ni.elem(arma::regspace<arma::uvec>(k - k0, 1, k - 1)).as_row());
    
    
    /// fit model with imputed data
    auto res = FixedEffectEstimator::FixedEffect(yi_fill, vi_fill);
    imputed_beta = res.est;
    imputed_pval = res.pval;
    
  } else {
    
    /// Rerpoting without the imputed results
    
  }
  
  /// \todo need to be integrated!
  //  if (estimator.find("R0") != std::string::npos) {
  //    m = -1:(k0-1)
  //    res$p.k0 = 1 - sum(choose(0+m+1, m+1) * 0.5^(0+m+2))
  //  } else {
  //    res$p.k0 = NA
  //  }
  
  /// \TODO Need to report the imputed stuff as well
  return ResultType{.k0 = k0, .se_k0 = se_k0, .k_all = k + k0, .side = 1};
  
}
