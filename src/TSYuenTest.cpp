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
  }
}

YuenTest::ResultType YuenTest::yuen_t_test_one_sample(
    const arma::Row<float> &x, float alpha,
    const TestStrategy::TestAlternative alternative, float trim = 0.2,
    float mu = 0.0) {

  float M{0};

  bool sig{false};
  float Sm1 = arma::mean(x);

  auto n = x.n_elem;

  int g = static_cast<int>(floor(trim * n));

  float df = n - 2 * g - 1;

  float sw = sqrt(win_var(x, trim));

  float se = sw / ((1. - 2. * trim) * sqrt(n));

  float dif = trim_mean(x, trim);

  float t_stat = (dif - mu) / se;

  students_t dist(df);
  float p = 0;

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

  return {.tstat = t_stat, .df = df, .pvalue = p, .sig = sig};
}

YuenTest::ResultType
YuenTest::yuen_t_test_paired(const arma::Row<float> &x,
                             const arma::Row<float> &y, float alpha,
                             const TestStrategy::TestAlternative alternative,
                             float trim = 0.2, float mu = 0) {
  // Do some check whether it's possible to run the test

  float Sm1 = arma::mean(x);
  float Sm2 = arma::mean(y);

  bool sig{false};

  auto h1 = x.n_elem - 2 * static_cast<int>(floor(trim * x.n_elem));

  float q1 = (x.n_elem - 1) * win_var(x, trim);

  float q2 = (y.n_elem - 1) * win_var(y, trim);

  float q3 = (x.n_elem - 1) * std::get<1>(win_cor_cov(x, y, trim));

  float df = h1 - 1;

  float se = sqrt((q1 + q2 - 2 * q3) / (h1 * (h1 - 1)));

  float dif = trim_mean(x, trim) - trim_mean(y, trim);

  float t_stat = (dif - mu) / se;

  students_t dist(df);
  float p = 0;

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

  return {.tstat = t_stat, .df = df, .pvalue = p, .sig = sig};
}

YuenTest::ResultType YuenTest::yuen_t_test_two_samples(
    const arma::Row<float> &x, const arma::Row<float> &y, float alpha,
    const TestStrategy::TestAlternative alternative, float trim, float mu) {

  float Sm1 = arma::mean(x);
  float Sm2 = arma::mean(y);

  bool sig{false};

  int h1 = x.n_elem - 2 * floor(trim * x.n_elem);
  int h2 = y.n_elem - 2 * floor(trim * y.n_elem);

  float d1 = (x.n_elem - 1.) * win_var(x, trim) / (h1 * (h1 - 1.));
  float d2 = (y.n_elem - 1.) * win_var(y, trim) / (h2 * (h2 - 1.));

  if (!(isgreater(d1, 0) or isless(d1, 0))) {
    // Samples are almost equal and elements are constant
    d1 += std::numeric_limits<float>::epsilon();
  }

  if (!(isgreater(d2, 0) or isless(d2, 0))) {
    // Samples are almost equal and elements are constant
    d2 += std::numeric_limits<float>::epsilon();
  }

  float df =
      pow(d1 + d2, 2) / (pow(d1, 2) / (h1 - 1.) + pow(d2, 2) / (h2 - 1.));

  float se = sqrt(d1 + d2);

  float dif = trim_mean(x, trim) - trim_mean(y, trim);

  float t_stat = (dif - mu) / se;

  students_t dist(df);
  float p;

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

  return {.tstat = t_stat, .df = df, .pvalue = p, .sig = sig};
}
