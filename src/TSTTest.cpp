//
// Created by Amir Masoud Abdol on 2020-04-11
//

#include "TestStrategy.h"

#include <boost/math/distributions/students_t.hpp>

using namespace sam;
using boost::math::students_t;

void TTest::run(Experiment *experiment) {

  static ResultType res{};

  // The first group is always the control group
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {

    if (params.var_equal) {
      res = two_samples_t_test_equal_sd((*experiment)[d].mean_, (*experiment)[d].stddev_,
                                        (*experiment)[d].nobs_, (*experiment)[i].mean_,
                                        (*experiment)[i].stddev_, (*experiment)[i].nobs_,
                                        params.alpha, params.alternative);
    } else {
      res = two_samples_t_test_unequal_sd((*experiment)[d].mean_, (*experiment)[d].stddev_,
                                          (*experiment)[d].nobs_, (*experiment)[i].mean_,
                                          (*experiment)[i].stddev_, (*experiment)[i].nobs_,
                                          params.alpha,
                                          params.alternative);
    }

    (*experiment)[i].stats_ = res.tstat;
    (*experiment)[i].pvalue_ = res.pvalue;
    (*experiment)[i].eff_side_ = res.side;
    (*experiment)[i].sig_ = res.sig;
  }
}

TTest::ResultType TTest::t_test(const arma::Row<float> &dt1,
                                const arma::Row<float> &dt2, float alpha,
                                TestStrategy::TestAlternative alternative) {
  return t_test(arma::mean(dt1), arma::stddev(dt1), dt1.size(), arma::mean(dt2),
                arma::stddev(dt2), dt2.size(), alpha, alternative, true);
}

TTest::ResultType TTest::t_test(float Sm1, float Sd1, float Sn1, float Sm2,
                                float Sd2, float Sn2, float alpha,
                                TestStrategy::TestAlternative alternative,
                                bool equal_var = false) {

  if (Sm1 == 0.) {
    return single_sample_t_test(Sm1, Sm2, Sd2, Sn2, alpha, alternative);
  }

  if (equal_var) {
    return two_samples_t_test_equal_sd(Sm1, Sd1, Sn1, Sm2, Sd2, Sn2, alpha,
                                       alternative);
  } else {
    return two_samples_t_test_unequal_sd(Sm1, Sd1, Sn1, Sm2, Sd2, Sn2, alpha,
                                         alternative);
  }
}

std::pair<float, bool>
TTest::compute_pvalue(float t_stat, float df, float alpha, TestStrategy::TestAlternative alternative) {
  
  using boost::math::students_t;
  students_t dist(df);
  
  float p;
  float sig;
  
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
    p = cdf(dist, t_stat);
    if (p > alpha) // Alternative "NOT REJECTED"
      sig = true;
    else // Alternative "REJECTED"
      sig = false;
  }
  
  if (alternative == TestStrategy::TestAlternative::Less) {
    // Mean  < M
    p = cdf(complement(dist, t_stat));
    if (p > alpha) // Alternative "NOT REJECTED"
      sig = true;
    else // Alternative "REJECTED"
      sig = false;
  }
  
  return std::make_pair(p, sig);
}

///
/// A Students t test applied to a single set of data. We are testing the null
/// hypothesis that the true mean of the sample is M, and that any variation is
/// down to chance.  We can also test the alternative hypothesis that any
/// difference is not down to chance
///
/// @note       Obtained from [Boost Library
///             Example](https://www.boost.org/doc/libs/1_69_0/libs/math/doc/html/math_toolkit/stat_tut/weg/st_eg/paired_st.html).
///
/// @param      M      True Mean.
/// @param      Sm     Sample Mean.
/// @param      Sd     Sample Standard Deviation.
/// @param      Sn     Sample Size.
/// @param      alpha  Significance Level.
/// @return     TTest::ResultType
///
TTest::ResultType
TTest::single_sample_t_test(float M, float Sm, float Sd, unsigned Sn,
                            float alpha,
                            TestStrategy::TestAlternative alternative) {

  bool sig = false;

  // Difference in means:
  float diff = Sm - M;

  // Degrees of freedom:
  float df = Sn - 1;

  // t-statistic:
  float t_stat = diff * sqrt(float(Sn)) / Sd;

  //
  // Finally define our distribution, and get the probability:
  //
  students_t dist(df);
  float p = 0;

  //
  // Finally print out results of alternative hypothesis:
  //

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
    p = cdf(dist, t_stat);
    if (p > alpha) // Alternative "NOT REJECTED"
      sig = true;
    else // Alternative "REJECTED"
      sig = false;
  }

  if (alternative == TestStrategy::TestAlternative::Less) {
    // Mean  < M
    p = cdf(complement(dist, t_stat));
    if (p > alpha) // Alternative "NOT REJECTED"
      sig = true;
    else // Alternative "REJECTED"
      sig = false;
  }

  int eff_side = std::copysign(1.0, M - Sm);

  return {.tstat = t_stat, .df = df, .pvalue = p, .side = eff_side, .sig = sig};
}

///
/// A Students t test applied to two sets of data. We are testing the null
/// hypothesis that the two samples have the same mean and that any difference
/// if due to chance.
///
/// @note       Obtained from [Boost Library
///             Example](https://www.boost.org/doc/libs/1_69_0/libs/math/doc/html/math_toolkit/stat_tut/weg/st_eg/paired_st.html).
///
/// @param      Sm1    Sample Mean 1.
/// @param      Sd1    Sample Standard Deviation 1.
/// @param      Sn1    Sample Size 1.
/// @param      Sm2    Sample Mean 2.
/// @param      Sd2    Sample Standard Deviation 2.
/// @param      Sn2    Sample Size 2.
/// @param      alpha  Significance Level.
/// @return     TTest::ResultType
///
TTest::ResultType TTest::two_samples_t_test_equal_sd(
    float Sm1, float Sd1, unsigned Sn1, float Sm2, float Sd2, unsigned Sn2,
    float alpha, TestStrategy::TestAlternative alternative) {

  bool sig = false;

  // Degrees of freedom:
  float df = Sn1 + Sn2 - 2;

  // Pooled variance and hence standard deviation:
  float sp = sqrt(((Sn1 - 1) * Sd1 * Sd1 + (Sn2 - 1) * Sd2 * Sd2) / df);

  // NOTE: I had to do this, I don't want my simulations fail.
  // While this is not perfect, it allows me to handle an edge case
  // SAM should not throw and should continue working.
  if (!(isgreater(sp, 0) or isless(sp, 0))) {
    // Samples are almost equal and elements are constant
    sp += std::numeric_limits<float>::epsilon();
  }

  // t-statistic:
  float t_stat = (Sm1 - Sm2) / (sp * sqrt(1.0 / Sn1 + 1.0 / Sn2));

  //
  // Define our distribution, and get the probability:
  //
  students_t dist(df);
  float p = 0;

  //
  // Finally print out results of alternative hypothesis:
  //

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
    else
      sig = false; // Alternative "REJECTED"
  }

  int eff_side = std::copysign(1.0, Sm2 - Sm1);

  return {.tstat = t_stat, .df = df, .pvalue = p, .side = eff_side, .sig = sig};
}

///
/// A Students t test applied to two sets of data with _unequal_ variance. We
/// are testing the null hypothesis that the two samples have the same mean and
/// that any difference is due to chance.
///
/// @note       Obtained from [Boost Library
///             Example](https://www.boost.org/doc/libs/1_69_0/libs/math/doc/html/math_toolkit/stat_tut/weg/st_eg/paired_st.html).
///
/// @param      Sm1    Sample Mean 1.
/// @param      Sd1    Sample Standard Deviation 1.
/// @param      Sn1    Sample Size 1.
/// @param      Sm2    Sample Mean 2.
/// @param      Sd2    Sample Standard Deviation 2.
/// @param      Sn2    Sample Size 2.
/// @param      alpha  Significance Level.
/// @return     TTest::ResultType
///
TTest::ResultType TTest::two_samples_t_test_unequal_sd(
    float Sm1, float Sd1, unsigned Sn1, float Sm2, float Sd2, unsigned Sn2,
    float alpha, TestStrategy::TestAlternative alternative) {

  bool sig = false;

  // Degrees of freedom:
  float df = Sd1 * Sd1 / Sn1 + Sd2 * Sd2 / Sn2;
  df *= df;
  float t1 = Sd1 * Sd1 / Sn1;
  t1 *= t1;
  t1 /= (Sn1 - 1);
  float t2 = Sd2 * Sd2 / Sn2;
  t2 *= t2;
  t2 /= (Sn2 - 1);
  df /= (t1 + t2);

  // t-statistic:
  float t_stat = (Sm1 - Sm2) / sqrt(Sd1 * Sd1 / Sn1 + Sd2 * Sd2 / Sn2);

  //
  // Define our distribution, and get the probability:
  //
  students_t dist(df);
  float p = 0;

  //
  // Finally print out results of alternative hypothesis:
  //

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
    else
      sig = false; // Alternative "REJECTED"
  }

  int eff_side = std::copysign(1.0, Sm2 - Sm1);

  return {.tstat = t_stat, .df = df, .pvalue = p, .side = eff_side, .sig = sig};
}
