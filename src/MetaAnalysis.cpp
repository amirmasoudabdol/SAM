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
#include <boost/math/special_functions/gamma.hpp>
#include <boost/math/special_functions/binomial.hpp>

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
  }else if (name == "RankCorrelation") {
    return std::make_unique<RankCorrelation>();
  }else{
    throw std::invalid_argument("Invalid Meta Analysis Strategy.");
  }
}

std::unique_ptr<MetaAnalysis> MetaAnalysis::build(const json &config) {
  if (config["name"] == "FixedEffectEstimator") {
    return std::make_unique<FixedEffectEstimator>();
  }else if (config["name"] == "RandomEffectEstimator") {
    
    auto p = config.get<RandomEffectEstimator::Parameters>();
    return std::make_unique<RandomEffectEstimator>(p);
    
  }else if (config["name"] == "EggersTestEstimator") {
    auto p = config.get<EggersTestEstimator::Parameters>();
    return std::make_unique<EggersTestEstimator>(p);
    
  }else if (config["name"] == "TestOfObsOverExptSig") {
    auto p = config.get<TestOfObsOverExptSig::Parameters>();
    return std::make_unique<TestOfObsOverExptSig>(p);
    
  }else if (config["name"] == "TrimAndFill") {
    auto p = config.get<TrimAndFill::Parameters>();
    return std::make_unique<TrimAndFill>(p);
    
  }else if (config["name"] == "RankCorrelation") {
    auto p = config.get<RankCorrelation::Parameters>();
    return std::make_unique<RankCorrelation>(p);
    
  }else{
    throw std::invalid_argument("Invalid Meta Analysis Strategy.");
  }
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
  }else if (name == "RankCorrelation") {
    return RankCorrelation::ResultType::Columns();
  }else{
    throw std::invalid_argument("Invalid Meta Analysis Strategy.");
  }
}

void FixedEffectEstimator::estimate(Journal *journal) {
  spdlog::debug("Computing Fixed Effect Estimate...");
  
  journal->meta_analysis_submissions.push_back(FixedEffect(journal->yi, journal->vi));
}

void RandomEffectEstimator::estimate(Journal *journal) {
  
  spdlog::debug("Computing Random Effect Estimate...");
  
  double tau2 {0};
  
  if (params.estimator.find("DL") != std::string::npos){
    tau2 = RandomEffectEstimator::DL(journal->yi, journal->vi, journal->wi);
  }else if (params.estimator.find("DL") != std::string::npos){
    throw std::invalid_argument("Not implemented yet!");
//    tau2 = RandomEffectEstimator::PM(journal->yi, journal->vi, tau2);
  }
  
  journal->meta_analysis_submissions.push_back(RandomEffect(journal->yi, journal->vi, tau2));
}


void EggersTestEstimator::estimate(Journal *journal) {
  
  spdlog::debug("Computing Eggers Estimate...");
  
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
  
  spdlog::debug("→ Estimating the tau2 using DL ...");
  
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

sam::TestOfObsOverExptSig::ResultType
TestOfObsOverExptSig::TES(const arma::Row<double> &sigs, const arma::Row<double> &ni, double beta, double alpha) {
  
  using boost::math::students_t;
  using boost::math::non_central_t;
  using boost::math::chi_squared;
  
  double k = sigs.n_elem;
  
  double O = arma::accu(sigs);

  arma::rowvec tcvs(k);
  tcvs.imbue([&, i = 0]() mutable {
    students_t tdist(ni[i] - 1); i++;
    return quantile(tdist, 0.95);
  });
  
  // non-central t-statistics
  arma::rowvec powers(k);
  powers.imbue([&, i = 0]() mutable {
    non_central_t nct(ni[i] - 1, beta * sqrt(ni[i]));
    return cdf(complement(nct, tcvs[i++]));
  });
  
  double E = arma::accu(powers);
  
  // A is most likely different from what R spit out, due to brutal rounding that's happening in R.
  double A = pow(O - E, 2.) / E + pow(O - E, 2.) / (k - E);
  
  chi_squared chisq(1);
  double pval = cdf(complement(chisq, A));
  /// @bug Sometimes, this happens, but I'm not exactly sure when and why
  ///  libc++abi.dylib: terminating with uncaught exception of type boost::wrapexcept<std::domain_error>: Error in function boost::math::cdf(const chi_squared_distribution<double>&, double): Chi Square parameter was nan, but must be > 0
  ///
  /// I've modified the implementation a bit and that might resolve this too, but I'll leave the flag for now
  
  
  return TestOfObsOverExptSig::ResultType{E, A, pval, pval < alpha};
}


void TestOfObsOverExptSig::estimate(Journal *journal) {
  
  spdlog::debug("Computing Test Of Obs Over Expt Significance...");
  
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
  
  spdlog::debug("Computing Trim And Fill...");
  
  arma::rowvec ni(journal->yi.n_elem);
  ni.imbue([&, i = 0]() mutable {
    return journal->publications_list[i++].group_.nobs_;
  });
  
  journal->meta_analysis_submissions.push_back(TrimAndFill::TF(journal->yi, journal->vi, ni, params));
}

TrimAndFill::ResultType TrimAndFill::TF(arma::Row<double> yi, arma::Row<double> vi, arma::Row<double> ni, const Parameters &params) {
  
  int k = yi.n_elem;
  arma::rowvec wi = 1. / vi;
  
  std::string side = params.side;

  /// Determining the side
  double beta = FixedEffectEstimator::FixedEffect(yi, vi).est;
  
  if (params.side.find("auto") != std::string::npos) {
    if (beta < 0) {
      side = "right";
    } else {
      side = "left";
    }
  }
  
  /// flip data if examining right side
  if (side.find("right") != std::string::npos){
    yi = -1. * yi;
  }
  
  /// sort data by increasing yi
  arma::uvec ix = arma::sort_index(yi);
  arma::rowvec yi_s = yi.elem(ix).as_row();
  arma::rowvec vi_s = vi.elem(ix).as_row();
  arma::rowvec wi_s = wi.elem(ix).as_row();
  arma::rowvec ni_s = wi.elem(ix).as_row();
  
  int iter{0};
  int maxiter{100};
  
  double k0_sav{-1};
  double k0{0}; // estimated number of missing studies;
  double se_k0{0};
  double Sr{0};
  double varSr{0};
  double k0_pval{0};
  
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
    
    yi_c     = yi_s - beta;                             ///  centered values;
    yi_c_r   = rankdata(abs(yi_c), "average").as_row(); /// \todo ties_method="first"); //  ranked absolute centered values;
    yi_c_r_s = arma::sign(yi_c) % yi_c_r;               ///  signed ranked centered values;
    
    //  estimate the number of missing studies with the R0 estimator
    
    if (params.estimator.find("R0") != std::string::npos) {
      arma::uvec inx = arma::find(yi_c_r_s < 0);
      k0 = (k - arma::max(-1. * yi_c_r_s.elem(inx))) - 1;
      se_k0 = sqrt(2 * std::max(0., k0) + 2);
    }
    
    ///  estimate the number of missing studies with the L0 estimator
    if (params.estimator.find("L0") != std::string::npos) {
      arma::uvec inx = arma::find(yi_c_r_s > 0);
      Sr = arma::accu(yi_c_r_s.elem(inx));
      k0 = (4.*Sr - k*(k+1.)) / (2.*k - 1.);
      varSr = 1./24 * (k*(k+1.)*(2.*k+1.) + 10.*pow(k0,3) + 27.*pow(k0,2) + 17.*k0 - 18.*k*pow(k0,2) - 18.*k*k0 + 6.*pow(k,2)*k0);
      se_k0 = 4.*sqrt(varSr) / (2*k - 1);
    }
    
    ///  estimate the number of missing studies with the Q0 estimator
    if (params.estimator.find("Q0") != std::string::npos) {
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
  
  auto res = FixedEffectEstimator::FixedEffect(yi, vi);
  double imputed_est = res.est;
  double imputed_pval = res.pval;
  
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
    imputed_est = res.est;
    imputed_pval = res.pval;
    
  }
    
  /// \todo need to be integrated!
  std::optional<double> p_k0;
  
  /// Adjustment for p_k0
  if (params.estimator.find("R0") != std::string::npos) {
    arma::rowvec m {arma::regspace<arma::rowvec>(-1, 1, (k0-1))};
    arma::rowvec bin_coefs(m.n_elem);
    /// \todo This imbue can be improved
    bin_coefs.imbue([&, i = 0]() mutable {
      auto x = boost::math::binomial_coefficient<double>(0+m.at(i)+1, m.at(i)+1);
      i++;
      return x;
    });
    arma::rowvec tmp(m.n_elem);
    tmp.imbue([&, i = 0]() mutable {
      return pow(0.5, static_cast<int>(0 + m.at(i++) + 2));
    });
    p_k0 = 1 - arma::accu(bin_coefs % tmp);
  } //else
    // p_k0 = NA
  
  /// \todo Still need to report the p_k0
  return ResultType{.k0 = k0, .se_k0 = se_k0, .k_all = k + k0, .side = side, .imputed_est = imputed_est, .imputed_pval = imputed_pval};
  
}

namespace sam {

/*-------------------------------------------------------------------------
 * This function calculates the Kendall correlation tau_b.
 *
 * from: https://afni.nimh.nih.gov/pub/dist/src/ktaub.c
 */
double kendallcor(const arma::Row<double> &x, const arma::Row<double> &y) {
  
  spdlog::debug(" → Computing Kendall Correlation...");
  
  int len = x.n_elem;
  
  int m1 = 0, m2 = 0, s = 0, nPair , i,j ;
  float cor ;
  
  for(i = 0; i < len; i++) {
    for(j = i + 1; j < len; j++) {
      if(y[i] > y[j]) {
        if (x[i] > x[j]) {
          s++;
        } else if(x[i] < x[j]) {
          s--;
        } else {
          m1++;
        }
      } else if(y[i] < y[j]) {
        if (x[i] > x[j]) {
          s--;
        } else if(x[i] < x[j]) {
          s++;
        } else {
          m1++;
        }
      } else {
        m2++;
        
        if(x[i] == x[j]) {
          m1++;
        }
      }
    }
  }
  
  nPair = len * (len - 1) / 2;
  
  if( m1 < nPair && m2 < nPair )
    cor = s / ( sqrtf((float)(nPair-m1)) * sqrtf((float)(nPair-m2)) );
  else
    cor = 0.0f;
  
  return cor;
}

double ckendall(int k, int n, arma::mat &w) {
  int i, u;
  double s;
  
  u =  (n * (n - 1) / 2);
  if ((k < 0) || (k > u))
    return(0);
  
  if (w.at(n, k) < 0) {
    if (n == 1)
      w.at(n, k) = (k == 0);
    else {
      s = 0;
      for (i = 0; i < n; i++)
        s += ckendall(k - i, n - 1, w);
      w.at(n, k) = s;
    }
  }
  return(w.at(n, k));
}

double pkendall(int len, int n) {
  
  spdlog::debug(" → Computing Kendall Probability...");
  
  int i, j;
  double p, q;
  
  p = 0;
  q = len;
  
  size_t u =  (n * (n - 1) / 2);
  arma::mat w(n, u); w.fill(-1);

    if (q < 0)
      p = 0;
    else if (q > (n * (n - 1) / 2))
      p = 1;
    else {
      p = 0;
      for (j = 0; j <= q; j++)
        p += ckendall(j, n, w);
      p = p / boost::math::tgamma(n + 1);
    }
  
  spdlog::trace(" → → p = {:f}\n", p);
  return p;
}

std::pair<double, double> kendall_cor_test(const arma::Row<double> &x, const arma::Row<double> &y, const TestStrategy::TestAlternative alternative) {
  
  spdlog::debug(" → Running Kendall Correlation Test...");
  
  auto n = x.n_elem;
  auto r = kendallcor(x, y);
  
  auto q = round((r + 1.) * n * (n - 1.) / 4.);
  
  arma::rowvec x_uqniues = arma::unique(x);
  size_t x_n_uqniues = x_uqniues.n_elem;
  arma::rowvec y_uqniues = arma::unique(y);
  size_t y_n_uqniues = y_uqniues.n_elem;
  
  bool ties = (min(x_n_uqniues, y_n_uqniues) < n);

  double p{0};
  double statistic;
  
  if (!ties) {
    
    statistic = q;
    spdlog::trace(" → → Statistic: {}", q);
    
    switch (alternative) {
      case TestStrategy::TestAlternative::TwoSided: {
        if(q > n * (n - 1) / 4){
          p = 1 - pkendall(q - 1, n);
        }else{
          p = pkendall(q, n);
        }
        p = std::min(2. * p, 1.);
      }; break;
      case TestStrategy::TestAlternative::Greater: {
        p = 1. - pkendall(q - 1, n);
      }; break;
      case TestStrategy::TestAlternative::Less: {
        p = pkendall(q, n);
      }; break;
    }
    
  }else{
    /// @note I'm not 100% sure if this is a good replacement for `table` but it seems to
    /// be working!
    spdlog::debug("Found ties...");
    spdlog::warn("Cannot compute exact p-value with ties!");
    
    /// xties <- table(x[duplicated(x)]) + 1;
    arma::urowvec xties;
    if (x_n_uqniues > 0) {
      xties = arma::hist(x, arma::sort(arma::unique(x))) - 1;
      xties = arma::nonzeros(xties).as_row() + 1;
    }else
      xties = arma::urowvec({0});
    
    /// yties <- table(y[duplicated(y)]) + 1;
    arma::urowvec yties;
    if (y_n_uqniues) {
      yties = arma::hist(y, arma::sort(arma::unique(y))) - 1;
      yties = arma::nonzeros(yties).as_row() + 1;
    }else
      yties = arma::urowvec({0});
    
    double T0 = n * (n - 1)/2;
    
    double T1 = arma::accu(xties % (xties - 1))/2;
    
    double T2 = arma::accu(yties % (yties - 1))/2;
    
    double S = r * sqrt((T0 - T1) * (T0 - T2));
    
    double v0 = n * (n - 1) * (2 * n + 5);
    
    double vt = arma::accu(xties % (xties - 1) % (2 * xties + 5));
    
    double vu = arma::accu(yties % (yties - 1) % (2 * yties + 5));
    
    double v1 = arma::accu((xties % (xties - 1))) * arma::accu(yties % (yties - 1));
    
    double v2 = arma::accu((xties % (xties - 1)) % (xties - 2)) * arma::accu(yties % (yties - 1) % (yties - 2));
    
    double var_S = (v0 - vt - vu) / 18. + v1 / (2. * n * (n - 1.)) + v2 / (9. * n * (n - 1.) * (n - 2.));
    
    statistic = S / sqrt(var_S);
    
    using boost::math::normal;
    normal norm;
    
    /// \todo check if these are what I want
    switch (alternative) {
      case TestStrategy::TestAlternative::TwoSided: {
        p = 2 * min(cdf(norm, statistic), cdf(complement(norm, statistic)));
      }; break;
      case TestStrategy::TestAlternative::Greater: {
        p = cdf(complement(norm, statistic));
      }; break;
      case TestStrategy::TestAlternative::Less: {
        p = cdf(norm, statistic);
      }; break;
    }
    
  }
  
  
  
  
  return std::make_pair(r, p);
}

}

RankCorrelation::ResultType RankCorrelation::RankCor(arma::Row<double> yi, arma::Row<double> vi, const Parameters &params) {
  
  auto res  = FixedEffectEstimator::FixedEffect(yi, vi);
  auto beta = res.est;
    
  auto k = yi.n_elem;
  arma::vec X = yi.as_col();
  arma::vec wi = 1./vi.as_col();
  arma::mat W = arma::diagmat(wi);
  arma::mat sWX = arma::sqrt(W) * X;
  arma::mat res_qrs = arma::solve(sWX, arma::diagmat(arma::vec(k, arma::fill::ones)));
  double vb = arma::as_scalar(res_qrs * res_qrs.t());
  
  arma::rowvec vi_star = vi - vb;
  arma::rowvec yi_star = (yi - beta) / arma::sqrt(vi_star);
  auto ken_res = kendall_cor_test(yi_star, vi, params.alternative);
  
  auto tau  = ken_res.first;
  auto pval = ken_res.second;
  spdlog::trace("Kendal Correlation Test: tau: {}, p: {}", tau, pval);
  
  
  return {.est = tau, .pval = pval, .sig = pval < params.alpha};
  
  
}

void RankCorrelation::estimate(sam::Journal *journal) { 
  journal->meta_analysis_submissions.push_back(RankCorrelation::RankCor(journal->yi, journal->vi, params));
}

