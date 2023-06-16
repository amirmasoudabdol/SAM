//
// Created by Amir Masoud Abdol on 2019-04-25.
//

#include <algorithm>
#include <cmath>

#include <spdlog/spdlog.h>
#include <fmt/core.h>

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
  
}

std::unique_ptr<MetaAnalysis> MetaAnalysis::build(std::string name) {
  
  spdlog::debug("Building a Meta Analysis Method");
  
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
    spdlog::critical("Invalid Meta Analysis Strategy.");
    exit(1);
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
    spdlog::critical("Invalid Meta Analysis Strategy.");
    exit(1);
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
    spdlog::critical("Invalid Meta Analysis Strategy.");
    exit(1);
  }
}

void FixedEffectEstimator::estimate(Journal *journal) {
  spdlog::debug("Computing Fixed Effect Estimate...");
  
  journal->storeMetaAnalysisResult(FixedEffect(journal->yi, journal->vi));
}

void RandomEffectEstimator::estimate(Journal *journal) {
  
  spdlog::debug("Computing Random Effect Estimate...");
  
  float tau2 {0};
  
  if (params.estimator.find("DL") != std::string::npos){
    tau2 = RandomEffectEstimator::DL(journal->yi, journal->vi, journal->wi);
  }else if (params.estimator.find("PM") != std::string::npos){
    spdlog::critical("Not implemented yet!");
    exit(1);
//    tau2 = RandomEffectEstimator::PM(journal->yi, journal->vi, tau2);
  }
  
  journal->storeMetaAnalysisResult(RandomEffect(journal->yi, journal->vi, tau2));
}


void EggersTestEstimator::estimate(Journal *journal) {
  
  spdlog::debug("Computing Eggers Estimate...");
  
  journal->storeMetaAnalysisResult(EggersTest(journal->yi, journal->vi, params.alpha));
}


RandomEffectEstimator::ResultType
RandomEffectEstimator::RandomEffect(const arma::Row<float> &yi, const arma::Row<float> &vi, float tau2) {
  
  using boost::math::normal;
  using boost::math::chi_squared;
  
  normal norm(0, 1);
  
  // Weight per study
  arma::Row<float> wi = 1. / (vi + tau2);
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
  
  arma::Row<float> wi_fe = 1. / vi;
  auto est_fe = arma::accu(wi_fe % yi)/ arma::accu(wi_fe);
  
  // Q-statistic
  auto q_stat = arma::accu(wi_fe % arma::pow(yi - est_fe, 2));
  
  chi_squared chisq(yi.n_elem - 1);
  // p-value of Q-statistic
  auto q_pval = cdf(complement(chisq, q_stat));
  
  return ResultType{est, static_cast<float>(se), static_cast<float>(ci_lb), static_cast<float>(ci_ub), static_cast<float>(zval), static_cast<float>(pval), q_stat, static_cast<float>(q_pval), tau2};
}


// General method-of-moments estimate (Eq. 6 in DerSimonian and Kacker, 2007)
float RandomEffectEstimator::DL(const arma::Row<float> &yi, const arma::Row<float> &vi, const arma::Row<float> &wi) {
  
  spdlog::trace("→ Estimating the tau2 using DL ...");
  
  auto q = arma::accu(wi % arma::pow(yi - (arma::accu(wi % yi)/arma::accu(wi)), 2));
  // spdlog::trace("Q: {}", q);
  
  auto tau2 = (q - (yi.n_elem - 1)) / (arma::accu(wi) - (arma::accu(arma::pow(wi, 2))/arma::accu(wi)));
  // spdlog::trace("Tau2: {}", tau2);
  
  tau2 = tau2 < 0 ? 0 : tau2;
  
  return tau2;
}

// Function for estimating tau2 with Paule-Mandel estimator
float RandomEffectEstimator::PM(const arma::Row<float> &yi, const arma::Row<float> &vi, const float tau2) {
  // Degrees of freedom of Q-statistic (df is also expected value because chi square distributed)
  auto df = yi.n_elem - 1;
  // Weights in meta-analysis
  arma::Row<float> wi = 1. / (vi + tau2);
  // Meta-analytic effect size
  auto theta = arma::accu(yi % wi)/arma::accu(wi);
  // Q-statistic
  auto Q = arma::accu(wi % arma::pow(yi - theta, 2));
  
  // Stop iterating if computed Q-statistic equals degrees of freedom
  
  return (Q - df);
}

EggersTestEstimator::ResultType
EggersTestEstimator::EggersTest(const arma::Row<float> &yi, const arma::Row<float> &vi, float alpha) {
  
  using namespace mlpack;
  
  using boost::math::students_t;
  
  arma::Row<double> Yi = arma::conv_to<arma::Row<double>>::from(yi);
  arma::Row<double> Vi = arma::conv_to<arma::Row<double>>::from(vi);
  
  auto n = Yi.n_elem;
  auto p = 2;
  float df = n - p;
  
  arma::Row<double> Wi = 1./Vi;
  arma::Row<double> wts = arma::sqrt(Wi);
  arma::Row<double> si = arma::sqrt(Vi);
  
  arma::Row<double> predictions(n);
  
  arma::Mat<double> X;
  X.ones(2, n);
  X.row(1) = si;
  
  LinearRegression lg(X, Yi, Wi);
  lg.Train(X, Yi, Wi, false);
  lg.Predict(X, predictions);
  
  arma::Row<double> errors = Yi - predictions;
  
  auto slope = lg.Parameters().at(1);
  
  arma::Mat<double> W = arma::diagmat(Wi);
  
  double res_var_2 = sqrt(arma::accu(Wi % arma::pow(errors, 2)) / (n - 2));
  arma::Mat<double> S_2 = arma::diagmat(arma::pow(res_var_2 / sqrt(Wi), 2));
  
  arma::Mat<double> Z = X.t();
  arma::Mat<double> var_betas = arma::sqrt(arma::inv(Z.t() * W * Z) * (Z.t() * W * S_2 * W.t() * Z) * arma::inv(Z.t() * W * Z));
  
  double slope_se = var_betas.diag().at(1);
  
  double slope_stat = slope / slope_se;
  
  auto res = TTest::compute_pvalue(slope_stat, n - 2, 0.1, TestStrategy::TestAlternative::TwoSided);
  
  return ResultType{static_cast<float>(slope), static_cast<float>(slope_se), static_cast<float>(slope_stat), res.first, res.second, df};
}

sam::TestOfObsOverExptSig::ResultType
TestOfObsOverExptSig::TES(const arma::Row<float> &sigs, const arma::Row<float> &ni, float beta, float alpha) {
  
  using boost::math::students_t;
  using boost::math::non_central_t;
  using boost::math::chi_squared;
  
  float k = sigs.n_elem;
  
  float O = arma::accu(sigs);

  arma::Row<float> tcvs(k);
  tcvs.imbue([&, i = 0]() mutable {
    students_t tdist(ni[i] - 1); i++;
    return quantile(tdist, 0.95);
  });
  
  // non-central t-statistics
  arma::Row<float> powers(k);
  powers.imbue([&, i = 0]() mutable {
    non_central_t nct(ni[i] - 1, beta * sqrt(ni[i]));
    return cdf(complement(nct, tcvs[i++]));
  });
  
  float E = arma::accu(powers);
  
  /// @note If E is absolute zero, I'm adding some noise that I don't have to deal with the explosion
  if (E < 0.0000001)
    E = 1e-10;
  
  /// A is most likely different from what R spit out, due to brutal rounding that's happening in R.
  float A {100000};
  float pval {0.0};
  if (k != E) {
    A = pow(O - E, 2.) / E + pow(O - E, 2.) / (k - E);
  
    if (!isnan(A) and !isinf(A)) {
      chi_squared chisq(1);
      pval = cdf(complement(chisq, A));
    }
  }
  
  return TestOfObsOverExptSig::ResultType{E, A, pval, pval < alpha};
}


void TestOfObsOverExptSig::estimate(Journal *journal) {
  
  spdlog::debug("Computing Test Of Obs Over Expt Significance...");
  
  float beta = FixedEffectEstimator::FixedEffect(journal->yi, journal->vi).est;
  
  arma::Row<float> sigs(journal->yi.n_elem);
  sigs.imbue([&, i = 0]() mutable {
    return journal->publications_list[i++].dv_.sig_;
  });
  
  arma::Row<float> ni(journal->yi.n_elem);
  ni.imbue([&, i = 0]() mutable {
    return journal->publications_list[i++].dv_.nobs_;
  });
  
  
  journal->storeMetaAnalysisResult(TestOfObsOverExptSig::TES(sigs, ni, beta, 0.05));
}

void TrimAndFill::estimate(Journal *journal) {
  
  spdlog::debug("Computing Trim And Fill...");
  
  arma::Row<float> ni(journal->yi.n_elem);
  ni.imbue([&, i = 0]() mutable {
    return journal->publications_list[i++].dv_.nobs_;
  });
  
  journal->storeMetaAnalysisResult(TrimAndFill::TF(journal->yi, journal->vi, ni, params));
}

TrimAndFill::ResultType TrimAndFill::TF(arma::Row<float> yi, arma::Row<float> vi, arma::Row<float> ni, const Parameters &params) {
  
  int k = yi.n_elem;
  arma::Row<float> wi = 1. / vi;
  
  std::string side = params.side;

  /// Determining the side
  float beta = FixedEffectEstimator::FixedEffect(yi, vi).est;
  
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
  arma::Row<float> yi_s = yi.elem(ix).as_row();
  arma::Row<float> vi_s = vi.elem(ix).as_row();
  arma::Row<float> wi_s = wi.elem(ix).as_row();
  arma::Row<float> ni_s = wi.elem(ix).as_row();
  
  int iter{0};
  int maxiter{100};
  
  float k0_sav{-1};
  float k0{0}; // estimated number of missing studies;
  float se_k0{0};
  float Sr{0};
  float varSr{0};
  float k0_pval{0};
  
  arma::Row<float> yi_c;
  arma::Row<float> yi_c_r;
  arma::Row<float> yi_c_r_s;
  
  while (abs(k0 - k0_sav) > 0) {
    
    k0_sav = k0; // save current value of k0;
    
    iter++;
    
    if (iter > maxiter)
      break;
    
    //  truncated data
    arma::uvec elems = arma::regspace<arma::uvec>(0, 1, k - k0 - 1);
    arma::Row<float> yi_t = yi_s.elem(elems).as_row();
    arma::Row<float> vi_t = vi_s.elem(elems).as_row();
    arma::Row<float> wi_t = wi_s.elem(elems).as_row();
    arma::Row<float> ni_t = wi_s.elem(elems).as_row();
    
    //  intercept estimate based on truncated data
    beta = FixedEffectEstimator::FixedEffect(yi_t, vi_t).est;
    
    yi_c     = yi_s - beta;                             ///  centered values;
    yi_c_r   = rankdata(abs(yi_c), "average").as_row(); /// @todo ties_method="first"); //  ranked absolute centered values;
    yi_c_r_s = arma::sign(yi_c) % yi_c_r;               ///  signed ranked centered values;
    
    //  estimate the number of missing studies with the R0 estimator
    
    if (params.estimator.find("R0") != std::string::npos) {
      arma::uvec inx = arma::find(yi_c_r_s < 0);
      k0 = (k - arma::max(-1. * yi_c_r_s.elem(inx))) - 1;
      se_k0 = sqrt(2 * std::max(static_cast<float>(0.), k0) + 2);
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
    k0 = std::max(static_cast<float>(0.), std::round(k0));
    se_k0 = std::max(static_cast<float>(0.), se_k0);
    
  }
  
  
  
  /// ------------------ Filling and estimating ----------------
  
  auto res = FixedEffectEstimator::FixedEffect(yi, vi);
  float imputed_est = res.est;
  float imputed_pval = res.pval;
  
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
    arma::Row<float> yi_f = yi_c;
    arma::Row<float> yi_fill = yi;
    yi_fill.insert_cols(yi_f.n_elem, -1. * yi_c.elem(arma::regspace<arma::uvec>(k - k0, 1, k - 1)).as_row());
    
    /// apply limits if specified
    /// @todo: to be implemented
    //    if (!missing(ilim)) {
    //      ilim = sort(ilim)
    //      if (length(ilim) != 2L)
    //        stop(mstyle$stop("Argument 'ilim' must be of length 2_"))
    //        yi_fill[yi_fill < ilim[1]] = ilim[1]
    //        yi_fill[yi_fill > ilim[2]] = ilim[2]
    //        }
    
    arma::Row<float> vi_fill = vi;
    vi_fill.insert_cols(vi.n_elem, vi.elem(arma::regspace<arma::uvec>(k - k0, 1, k - 1)).as_row());
    arma::Row<float> wi_fill = wi;
    wi_fill.insert_cols(wi.n_elem, wi.elem(arma::regspace<arma::uvec>(k - k0, 1, k - 1)).as_row());
    arma::Row<float> ni_fill = ni;
    ni_fill.insert_cols(ni.n_elem, ni.elem(arma::regspace<arma::uvec>(k - k0, 1, k - 1)).as_row());
    
    
    /// fit model with imputed data
    auto res = FixedEffectEstimator::FixedEffect(yi_fill, vi_fill);
    imputed_est = res.est;
    imputed_pval = res.pval;
    
  }
    
  /// @todo need to be integrated!
  std::optional<float> p_k0;
  
  /// Adjustment for p_k0
  if (params.estimator.find("R0") != std::string::npos) {
    arma::Row<float> m {arma::regspace<arma::Row<float>>(-1, 1, (k0-1))};
    arma::Row<float> bin_coefs(m.n_elem);
    /// @todo This imbue can be improved
    bin_coefs.imbue([&, i = 0]() mutable {
      auto x = boost::math::binomial_coefficient<float>(0+m.at(i)+1, m.at(i)+1);
      i++;
      return x;
    });
    arma::Row<float> tmp(m.n_elem);
    tmp.imbue([&, i = 0]() mutable {
      return pow(0.5, static_cast<int>(0 + m.at(i++) + 2));
    });
    p_k0 = 1 - arma::accu(bin_coefs % tmp);
  } //else
    // p_k0 = NA
  
  /// @todo Still need to report the p_k0
  return ResultType{.k0 = k0, .se_k0 = se_k0, .k_all = k + k0, .side = side, .imputed_est = imputed_est, .imputed_pval = imputed_pval};
  
}

namespace sam {

/*-------------------------------------------------------------------------
 * This function calculates the Kendall correlation tau_b.
 *
 * from: https://afni.nimh.nih.gov/pub/dist/src/ktaub.c
 */
float kendallcor(const arma::Row<float> &x, const arma::Row<float> &y) {
  
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

float ckendall(int k, int n, arma::Mat<float> &w) {
  int i, u;
  float s;
  
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

float pkendall(int len, int n) {
  
  spdlog::debug(" → Computing Kendall Probability...");
  
  int i, j;
  float p, q;
  
  p = 0;
  q = len;
  
  size_t u =  (n * (n - 1) / 2);
  arma::Mat<float> w(n, u); w.fill(-1);

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

std::pair<float, float> kendall_cor_test(const arma::Row<float> &x, const arma::Row<float> &y, const TestStrategy::TestAlternative alternative) {
  
  spdlog::debug(" → Running Kendall Correlation Test...");
  
  auto n = x.n_elem;
  auto r = kendallcor(x, y);
  
  auto q = round((r + 1.) * n * (n - 1.) / 4.);
  
  arma::Row<float> x_uqniues = arma::unique(x);
  size_t x_n_uqniues = x_uqniues.n_elem;
  arma::Row<float> y_uqniues = arma::unique(y);
  size_t y_n_uqniues = y_uqniues.n_elem;
  
  bool ties = (min(x_n_uqniues, y_n_uqniues) < n);

  float p{0};
  float statistic;
  
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
      } break;
      case TestStrategy::TestAlternative::Greater: {
        p = 1. - pkendall(q - 1, n);
      } break;
      case TestStrategy::TestAlternative::Less: {
        p = pkendall(q, n);
      } break;
    }
    
  }else{
    /// @note I'm not 100% sure if this is a good replacement for `table` but it seems to
    /// be working!
    spdlog::trace("Found ties...");
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
    
    float T0 = n * (n - 1)/2;
    
    float T1 = arma::accu(xties % (xties - 1))/2;
    
    float T2 = arma::accu(yties % (yties - 1))/2;
    
    float S = r * sqrt((T0 - T1) * (T0 - T2));
    
    float v0 = n * (n - 1) * (2 * n + 5);
    
    float vt = arma::accu(xties % (xties - 1) % (2 * xties + 5));
    
    float vu = arma::accu(yties % (yties - 1) % (2 * yties + 5));
    
    float v1 = arma::accu((xties % (xties - 1))) * arma::accu(yties % (yties - 1));
    
    float v2 = arma::accu((xties % (xties - 1)) % (xties - 2)) * arma::accu(yties % (yties - 1) % (yties - 2));
    
    float var_S = (v0 - vt - vu) / 18. + v1 / (2. * n * (n - 1.)) + v2 / (9. * n * (n - 1.) * (n - 2.));
    
    statistic = S / sqrt(var_S);
    
    using boost::math::normal;
    normal norm;
    
    /// @todo check if these are what I want
    switch (alternative) {
      case TestStrategy::TestAlternative::TwoSided: {
        p = 2 * min(cdf(norm, statistic), cdf(complement(norm, statistic)));
      } break;
      case TestStrategy::TestAlternative::Greater: {
        p = cdf(complement(norm, statistic));
      } break;
      case TestStrategy::TestAlternative::Less: {
        p = cdf(norm, statistic);
      } break;
    }
    
  }
  
  
  
  
  return std::make_pair(r, p);
}

}

RankCorrelation::ResultType RankCorrelation::RankCor(arma::Row<float> yi, arma::Row<float> vi, const Parameters &params) {
  
  auto res  = FixedEffectEstimator::FixedEffect(yi, vi);
  auto beta = res.est;
  auto vb = pow(res.se, 2);
  
  arma::Row<float> vi_star = vi - vb;
  arma::Row<float> yi_star = (yi - beta) / arma::sqrt(vi_star);
  
//  vi_star.replace(arma::datum::nan, 0.);
//  yi_star.replace(arma::datum::nan, 0.);
  auto ken_res = kendall_cor_test(yi_star, vi, params.alternative);
  
  auto tau  = ken_res.first;
  auto pval = ken_res.second;
  spdlog::trace("Kendal Correlation Test: tau: {}, p: {}", tau, pval);
  
  
  return {.est = tau, .pval = pval, .sig = pval < params.alpha};
  
  
}

void RankCorrelation::estimate(sam::Journal *journal) { 
  journal->storeMetaAnalysisResult(RankCorrelation::RankCor(journal->yi, journal->vi, params));
}

