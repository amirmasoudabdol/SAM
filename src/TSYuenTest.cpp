//
// Created by Amir Masoud Abdol on 2020-04-11
//

#include "TestStrategy.h"

#include <boost/math/distributions/students_t.hpp>

using namespace sam;
using boost::math::students_t;

void YuenTest::run(Experiment *experiment) {

  // The first group is always the control group

  static YuenTest::ResultType res;

  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {

    if (params.paired) {
      res = yuen_t_test_paired((*experiment)[d].measurements(),
                               (*experiment)[i].measurements(),
                               params.alpha,
                               params.alternative,
                               params.trim,
                               0);
    }else{
      res = yuen_t_test_two_samples((*experiment)[d].measurements(),
                                    (*experiment)[i].measurements(),
                                    params.alpha,
                                    params.alternative, params.trim, 0);
    }

    (*experiment)[i].stats_ = res.tstat;
    (*experiment)[i].pvalue_ = res.pvalue;
    (*experiment)[i].sig_ = res.sig;
    (*experiment)[i].eff_side_ = res.side;
  }
}

YuenTest::ResultType YuenTest::yuen_t_test_one_sample(
    const arma::Row<double> &x, double alpha,
    const TestStrategy::TestAlternative alternative, double trim = 0.2,
    double mu = 0.0) {

  double M{0};

  bool sig{false};
  double Sm1 = arma::mean(x);

  auto n = x.n_elem;

  int g = static_cast<int>(floor(trim * n));

  double df = n - 2 * g - 1;

  double sw = sqrt(win_var(x, trim));

  double se = sw / ((1. - 2. * trim) * sqrt(n));

  double dif = trim_mean(x, trim);

  double t_stat = (dif - mu) / se;

  students_t dist(df);
  double p = 0;

  if (alternative == TestStrategy::TestAlternative::TwoSided) {
    // Mean != M
    p = 2 * cdf(complement(dist, fabs(t_stat)));
    if (p < alpha) // Alternative "NOT REJECTED"
      sig = true;
    else // Alternative "REJECTED"
      sig = false;
  }

  if (alternative == TestStrategy::TestAlternative::Greater) {
    // Mean  > M
    p = cdf(complement(dist, t_stat));
    if (p > alpha) // Alternative "NOT REJECTED"
      sig = true;
    else // Alternative "REJECTED"
      sig = false;
  }

  if (alternative == TestStrategy::TestAlternative::Less) {
    // Mean  < M
    p = cdf(dist, t_stat);
    if (p > alpha) // Alternative "NOT REJECTED"
      sig = true;
    else // Alternative "REJECTED"
      sig = false;
  }

  int eff_side = std::copysign(1.0, Sm1 - M);

  return {.tstat = t_stat, .df = df, .pvalue = p, .side = eff_side, .sig = sig};
}

YuenTest::ResultType
YuenTest::yuen_t_test_paired(const arma::Row<double> &x,
                             const arma::Row<double> &y, double alpha,
                             const TestStrategy::TestAlternative alternative,
                             double trim = 0.2, double mu = 0) {
  // Do some check whether it's possible to run the test

  double Sm1 = arma::mean(x);
  double Sm2 = arma::mean(y);

  bool sig{false};

  auto h1 = x.n_elem - 2 * static_cast<int>(floor(trim * x.n_elem));

  double q1 = (x.n_elem - 1) * win_var(x, trim);

  double q2 = (y.n_elem - 1) * win_var(y, trim);

  double q3 = (x.n_elem - 1) * std::get<1>(win_cor_cov(x, y, trim));

  double df = h1 - 1;

  double se = sqrt((q1 + q2 - 2 * q3) / (h1 * (h1 - 1)));

  double dif = trim_mean(x, trim) - trim_mean(y, trim);

  double t_stat = (dif - mu) / se;

  students_t dist(df);
  double p = 0;

  if (alternative == TestStrategy::TestAlternative::TwoSided) {
    // Mean != M
    p = 2 * cdf(complement(dist, fabs(t_stat)));
    if (p < alpha) // Alternative "NOT REJECTED"
      sig = true;
    else // Alternative "REJECTED"
      sig = false;
  }

  if (alternative == TestStrategy::TestAlternative::Greater) {
    // Mean  > M
    p = cdf(complement(dist, t_stat));
    if (p > alpha) // Alternative "NOT REJECTED"
      sig = true;
    else // Alternative "REJECTED"
      sig = false;
  }

  if (alternative == TestStrategy::TestAlternative::Less) {
    // Mean  < M
    p = cdf(dist, t_stat);
    if (p > alpha) // Alternative "NOT REJECTED"
      sig = true;
    else // Alternative "REJECTED"
      sig = false;
  }

  int eff_side = std::copysign(1.0, Sm2 - Sm1);

  return {.tstat = t_stat, .df = df, .pvalue = p, .side = eff_side, .sig = sig};
}

YuenTest::ResultType YuenTest::yuen_t_test_two_samples(
    const arma::Row<double> &x, const arma::Row<double> &y, double alpha,
    const TestStrategy::TestAlternative alternative, double trim, double mu) {

  double Sm1 = arma::mean(x);
  double Sm2 = arma::mean(y);

  bool sig{false};

  int h1 = x.n_elem - 2 * floor(trim * x.n_elem);
  int h2 = y.n_elem - 2 * floor(trim * y.n_elem);

  double d1 = (x.n_elem - 1.) * win_var(x, trim) / (h1 * (h1 - 1.));
  double d2 = (y.n_elem - 1.) * win_var(y, trim) / (h2 * (h2 - 1.));

  if (!(isgreater(d1, 0) or isless(d1, 0))) {
    // Samples are almost equal and elements are constant
    d1 += std::numeric_limits<double>::epsilon();
  }

  if (!(isgreater(d2, 0) or isless(d2, 0))) {
    // Samples are almost equal and elements are constant
    d2 += std::numeric_limits<double>::epsilon();
  }

  double df =
      pow(d1 + d2, 2) / (pow(d1, 2) / (h1 - 1.) + pow(d2, 2) / (h2 - 1.));

  double se = sqrt(d1 + d2);

  double dif = trim_mean(x, trim) - trim_mean(y, trim);

  double t_stat = (dif - mu) / se;

  students_t dist(df);
  double p;

  if (alternative == TestStrategy::TestAlternative::TwoSided) {
    // Sample 1 Mean != Sample 2 Mean
    p = 2 * cdf(complement(dist, fabs(t_stat)));
    if (p < alpha) // Alternative "NOT REJECTED"
      sig = true;
    else // Alternative "REJECTED"
      sig = false;
  }

  if (alternative == TestStrategy::TestAlternative::Greater) {
    // Sample 1 Mean <  Sample 2 Mean
    p = cdf(dist, t_stat);
    if (p < alpha) // Alternative "NOT REJECTED"
      sig = true;
    else // Alternative "REJECTED"
      sig = false;
  }

  if (alternative == TestStrategy::TestAlternative::Less) {
    // Sample 1 Mean >  Sample 2 Mean
    p = cdf(complement(dist, t_stat));
    if (p < alpha) // Alternative "NOT REJECTED"
      sig = true;
    else // Alternative "REJECTED"
      sig = false;
  }

  int eff_side = std::copysign(1.0, Sm2 - Sm1);

  return {.tstat = t_stat, .df = df, .pvalue = p, .side = eff_side, .sig = sig};
}
