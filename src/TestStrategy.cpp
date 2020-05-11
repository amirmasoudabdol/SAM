//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <spdlog/spdlog.h>

#include <boost/math/distributions/fisher_f.hpp>
#include <boost/math/distributions/normal.hpp>
#include <boost/math/distributions/students_t.hpp>

#include "Experiment.h"
#include "TestStrategy.h"

using namespace sam;
using boost::math::students_t;

TestStrategy::~TestStrategy(){
    // Pure deconstructor
};

std::unique_ptr<TestStrategy> TestStrategy::build(json &test_strategy_config) {

  if (test_strategy_config["_name"] == "TTest") {

    auto params = test_strategy_config.get<TTest::Parameters>();
    return std::make_unique<TTest>(params);

  } else if (test_strategy_config["_name"] == "YuenTest") {

    auto params = test_strategy_config.get<YuenTest::Parameters>();
    return std::make_unique<YuenTest>(params);

  } else if (test_strategy_config["_name"] == "WilcoxonTest") {
    auto params = test_strategy_config.get<WilcoxonTest::Parameters>();
    return std::make_unique<WilcoxonTest>(params);
  } else {
    throw std::invalid_argument("Unknown Test Strategy.");
  }
}

void TTest::run(Experiment *experiment) {

  static TestStrategy::TestResult res;

  // The first group is always the control group
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {
    
    // This is not perfect, basically I need to check the population `vars`
    //        if ((isgreater(experiment->stddev[d], experiment->stddev[i]) or
    //        isless(experiment->stddev[d], experiment->stddev[i]))) {
    //
    //            res = two_samples_t_test_unequal_sd(experiment->means[d],
    //                                               experiment->stddev[d],
    //                                               experiment->measurements[d].size(),
    //                                               experiment->means[i],
    //                                               experiment->stddev[i],
    //                                               experiment->measurements[i].size(),
    //                                               params.alpha,
    //                                               params.alternative);
    //        }else {
    // EQUAL SD
    res = two_samples_t_test_equal_sd(
        (*experiment)[d].mean_, (*experiment)[d].stddev_,
        (*experiment)[d].nobs_, (*experiment)[i].mean_,
        (*experiment)[i].stddev_, (*experiment)[i].nobs_, params.alpha,
        params.alternative);

    //        }

    (*experiment)[i].stats_ = res.statistic;
    (*experiment)[i].pvalue_ = res.pvalue;
    (*experiment)[i].sig_ = res.sig;
    (*experiment)[i].eff_side_ = res.side;
  }
}

void YuenTest::run(Experiment *experiment) {

  // The first group is always the control group

  static TestStrategy::TestResult res;

  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {

    //        if (experiment->measurements[d].size() ==
    //        experiment->measurements[i].size()) {
    //
    //            res = yuen_t_test_paired(experiment->measurements[d],
    //                                       experiment->measurements[i],
    //                                       params.alpha,
    //                                       params.alternative,
    //                                       params.trim,
    //                                       0);
    //        }else{
    res = yuen_t_test_two_samples((*experiment)[d].measurements(),
                                  (*experiment)[i].measurements(), params.alpha,
                                  params.alternative, 0.2, 0);
    //        }

    (*experiment)[i].stats_ = res.statistic;
    (*experiment)[i].pvalue_ = res.pvalue;
    (*experiment)[i].sig_ = res.sig;
    (*experiment)[i].eff_side_ = res.side;
  }
}

void WilcoxonTest::run(Experiment *experiment) {

  // The first group is always the control group

  static TestStrategy::TestResult res;

  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {

    res = wilcoxon_test((*experiment)[d].measurements(),
                        (*experiment)[i].measurements(), 1, params.alpha,
                        params.alternative);

    (*experiment)[i].stats_ = res.statistic;
    (*experiment)[i].pvalue_ = res.pvalue;
    (*experiment)[i].sig_ = res.sig;
    (*experiment)[i].eff_side_ = res.side;
  }
}

namespace sam {

///
/// Calculate confidence intervals for the mean. For example if we set the
/// confidence limit to 0.95, we know that if we repeat the sampling 100 times,
/// then we expect that the true mean will be between out limits on 95 occations.
/// Note: this is not the same as saying a 95% confidence interval means that there
/// is a 95% probability that the interval contains the true mean. The interval
/// computed from a given sample either contains the true mean or it does not.
///
/// \note       Obtained from [Boost Library
///             Example](https://www.boost.org/doc/libs/1_69_0/libs/math/doc/html/math_toolkit/stat_tut/weg/st_eg/paired_st.html).
///
/// \param      Sm    Sample Mean.
/// \param      Sd    Sample Standard Deviation.
/// \param      Sn    Sample Size.
///
std::pair<double, double>
confidence_limits_on_mean(double Sm, double Sd, unsigned Sn, double alpha,
                          TestStrategy::TestAlternative alternative) {

  using namespace sam;

  students_t dist(Sn - 1);

  // calculate T
  double T =
      quantile(complement(dist, alpha / 2)); // TODO: Implement the side!./sa

  // Calculate width of interval (one sided):
  double w = T * Sd / sqrt(double(Sn));

  // Calculate and return the interval
  return std::make_pair(Sm - w, Sm + w);
}

///
/// Caculate the degress of freedom to achieve a significance result with the given
/// alpha
///
/// \param      M     True Mean.
/// \param      Sm    Sample Mean.
/// \param      Sd    Sample Standard Deviation.
///
double single_sample_find_df(double M, double Sm, double Sd, double alpha,
                             TestStrategy::TestAlternative alternative) {
  using namespace sam;
  using boost::math::students_t;

  // calculate df for one-sided or two-sided test:
  double df = students_t::find_degrees_of_freedom(
      fabs(M - Sm),
      (alternative == TestStrategy::TestAlternative::Greater) ? alpha
                                                              : alpha / 2.,
      alpha, Sd);

  // convert to sample size, always one more than the degrees of freedom:
  return ceil(df) + 1;
}

TestStrategy::TestResult t_test(const arma::Row<double> &dt1,
                                const arma::Row<double> &dt2, double alpha,
                                TestStrategy::TestAlternative alternative) {
  return t_test(arma::mean(dt1), arma::stddev(dt1), dt1.size(), arma::mean(dt2),
                arma::stddev(dt2), dt2.size(), alpha, alternative, true);
}

TestStrategy::TestResult t_test(double Sm1, double Sd1, double Sn1, double Sm2,
                                double Sd2, double Sn2, double alpha,
                                TestStrategy::TestAlternative alternative,
                                bool equal_var = false) {

  using namespace sam;

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

///
/// A Students t test applied to a single set of data. We are testing the null
/// hypothesis that the true mean of the sample is M, and that any variation is
/// down to chance.  We can also test the alternative hypothesis that any
/// difference is not down to chance
///
/// \note       Obtained from [Boost Library
///             Example](https://www.boost.org/doc/libs/1_69_0/libs/math/doc/html/math_toolkit/stat_tut/weg/st_eg/paired_st.html).
///
/// \param      M      True Mean.
/// \param      Sm     Sample Mean.
/// \param      Sd     Sample Standard Deviation.
/// \param      Sn     Sample Size.
/// \param      alpha  Significance Level.
/// \return     TestStrategy::TestResult
///
TestStrategy::TestResult
single_sample_t_test(double M, double Sm, double Sd, unsigned Sn, double alpha,
                     TestStrategy::TestAlternative alternative) {

  bool sig = false;

  // Difference in means:
  double diff = Sm - M;

  // Degrees of freedom:
  unsigned df = Sn - 1;

  // t-statistic:
  double t_stat = diff * sqrt(double(Sn)) / Sd;

  //
  // Finally define our distribution, and get the probability:
  //
  students_t dist(df);
  double p = 0;

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

  return {t_stat, p, eff_side, sig};
}

///
/// A Students t test applied to two sets of data. We are testing the null
/// hypothesis that the two samples have the same mean and that any difference if
/// due to chance.
///
/// \note       Obtained from [Boost Library
///             Example](https://www.boost.org/doc/libs/1_69_0/libs/math/doc/html/math_toolkit/stat_tut/weg/st_eg/paired_st.html).
///
/// \param      Sm1    Sample Mean 1.
/// \param      Sd1    Sample Standard Deviation 1.
/// \param      Sn1    Sample Size 1.
/// \param      Sm2    Sample Mean 2.
/// \param      Sd2    Sample Standard Deviation 2.
/// \param      Sn2    Sample Size 2.
/// \param      alpha  Significance Level.
/// \return     TestStrategy::TestResult
///
TestStrategy::TestResult
two_samples_t_test_equal_sd(double Sm1, double Sd1, unsigned Sn1, double Sm2,
                            double Sd2, unsigned Sn2, double alpha,
                            TestStrategy::TestAlternative alternative) {

  bool sig = false;

  // Degrees of freedom:
  double df = Sn1 + Sn2 - 2;

  // Pooled variance and hence standard deviation:
  double sp = sqrt(((Sn1 - 1) * Sd1 * Sd1 + (Sn2 - 1) * Sd2 * Sd2) / df);

  // NOTE: I had to do this, I don't want my simulations fail.
  // While this is not perfect, it allows me to handle an edge case
  // SAM should not throw and should continue working.
  if (!(isgreater(sp, 0) or isless(sp, 0))) {
    // Samples are almost equal and elements are constant
    sp += std::numeric_limits<double>::epsilon();
  }

  // t-statistic:
  double t_stat = (Sm1 - Sm2) / (sp * sqrt(1.0 / Sn1 + 1.0 / Sn2));

  //
  // Define our distribution, and get the probability:
  //
  students_t dist(df);
  double p = 0;

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

  return {t_stat, p, eff_side, sig};
}

///
/// A Students t test applied to two sets of data with _unequal_ variance. We are
/// testing the null hypothesis that the two samples have the same mean and that
/// any difference is due to chance.
///
/// \note       Obtained from [Boost Library
///             Example](https://www.boost.org/doc/libs/1_69_0/libs/math/doc/html/math_toolkit/stat_tut/weg/st_eg/paired_st.html).
///
/// \param      Sm1    Sample Mean 1.
/// \param      Sd1    Sample Standard Deviation 1.
/// \param      Sn1    Sample Size 1.
/// \param      Sm2    Sample Mean 2.
/// \param      Sd2    Sample Standard Deviation 2.
/// \param      Sn2    Sample Size 2.
/// \param      alpha  Significance Level.
/// \return     TestStrategy::TestResult
///
TestStrategy::TestResult
two_samples_t_test_unequal_sd(double Sm1, double Sd1, unsigned Sn1, double Sm2,
                              double Sd2, unsigned Sn2, double alpha,
                              TestStrategy::TestAlternative alternative) {

  bool sig = false;

  // Degrees of freedom:
  double df = Sd1 * Sd1 / Sn1 + Sd2 * Sd2 / Sn2;
  df *= df;
  double t1 = Sd1 * Sd1 / Sn1;
  t1 *= t1;
  t1 /= (Sn1 - 1);
  double t2 = Sd2 * Sd2 / Sn2;
  t2 *= t2;
  t2 /= (Sn2 - 1);
  df /= (t1 + t2);

  // t-statistic:
  double t_stat = (Sm1 - Sm2) / sqrt(Sd1 * Sd1 / Sn1 + Sd2 * Sd2 / Sn2);

  //
  // Define our distribution, and get the probability:
  //
  students_t dist(df);
  double p = 0;

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

  return {t_stat, p, eff_side, sig};
}

TestStrategy::TestResult f_test(double sd1,   // Sample 1 std deviation
                                double sd2,   // Sample 2 std deviation
                                double N1,    // Sample 1 size
                                double N2,    // Sample 2 size
                                double alpha) // Significance level
{

  using boost::math::fisher_f;

  bool sig{false};

  // F-statistic:
  double f_stats = (sd1 / sd2);

  //
  // Finally define our distribution, and get the probability:
  //
  fisher_f dist(N1 - 1, N2 - 1);
  double p = cdf(dist, f_stats);

  double ucv = quantile(complement(dist, alpha));
  double ucv2 = quantile(complement(dist, alpha / 2));
  double lcv = quantile(dist, alpha);
  double lcv2 = quantile(dist, alpha / 2);

  //
  // Finally print out results of null and alternative hypothesis:
  //
  if ((ucv2 < f_stats) || (lcv2 > f_stats)) // Alternative "NOT REJECTED"
    sig = true;
  else // Alternative "REJECTED"
    sig = false;

  if (lcv > f_stats) // Alternative "NOT REJECTED"
    sig = true;
  else // Alternative "REJECTED"
    sig = false;

  if (ucv < f_stats) // Alternative "NOT REJECTED"
    sig = true;
  else // Alternative "REJECTED"
    sig = false;

  //        int eff_side = std::copysign(1.0, Sm2 - Sm1);

  return {f_stats, p, 1, sig};
}

TestStrategy::TestResult
yuen_t_test_one_sample(const arma::Row<double> &x, double alpha,
                       const TestStrategy::TestAlternative alternative,
                       double trim = 0.2, double mu = 0.0) {

  double M{0};

  bool sig{false};
  double Sm1 = arma::mean(x);

  int n = x.n_elem;

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

  return {t_stat, p, eff_side, sig};
}

TestStrategy::TestResult
yuen_t_test_paired(const arma::Row<double> &x, const arma::Row<double> &y,
                   double alpha,
                   const TestStrategy::TestAlternative alternative,
                   double trim = 0.2, double mu = 0) {
  // Do some check whether it's possible to run the test

  double Sm1 = arma::mean(x);
  double Sm2 = arma::mean(y);

  bool sig{false};

  int h1 = x.n_elem - 2 * static_cast<int>(floor(trim * x.n_elem));

  double q1 = (x.n_elem - 1) * win_var(x, trim);

  double q2 = (y.n_elem - 1) * win_var(y, trim);

  double q3 = (x.n_elem - 1) * std::get<1>(win_cor_cov(x, y, trim));

  unsigned df = h1 - 1;

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

  return {t_stat, p, eff_side, sig};
}

TestStrategy::TestResult yuen_t_test_two_samples(
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

  unsigned df =
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

  return {t_stat, p, eff_side, sig};
}

double win_var(const arma::Row<double> &x, const double trim) {
  return arma::var(win_val(x, trim));
}

std::pair<double, double> win_cor_cov(const arma::Row<double> &x,
                                      const arma::Row<double> &y,
                                      const double trim) {

  arma::rowvec xvec{win_val(x, trim)};
  arma::rowvec yvec{win_val(y, trim)};

  arma::mat wcor{arma::cor(xvec, yvec)};
  double vwcor{wcor.at(0, 0)};

  arma::mat wcov{arma::cov(xvec, yvec)};
  double vwcov{wcov.at(0, 0)};

  return std::make_pair(vwcor, vwcov);
}

arma::Row<double> win_val(const arma::Row<double> &x, double trim) {

  arma::rowvec y{arma::sort(x)};

  int ibot = floor(trim * x.n_elem) + 1;
  int itop = x.n_elem - ibot + 1;

  double xbot{y.at(ibot - 1)};
  double xtop{y.at(itop - 1)};

  return arma::clamp(x, xbot, xtop);
}

// TODO: this can be an extention to arma, something like I did for
// nlohmann::json I should basically put it into arma's namespace
double trim_mean(const arma::Row<double> &x, double trim) {
  arma::rowvec y{arma::sort(x)};

  int ibot = floor(trim * x.n_elem) + 1;
  int itop = x.n_elem - ibot + 1;

  return arma::mean(y.subvec(ibot - 1, itop - 1));
}

TestStrategy::TestResult
wilcoxon_test(const arma::Row<double> &x, const arma::Row<double> &y,
              double alpha, double use_continuity,
              const TestStrategy::TestAlternative alternative) {

  using boost::math::normal;

  bool sig{false};

  double Sm1 = arma::mean(x);
  double Sm2 = arma::mean(y);

  // Getting the rank of the data
  arma::Row<double> xy = arma::join_rows(x, y);

  // ranked = rankdata(np.concatenate((x, y)));
  arma::vec ranked = rankdata(xy, "average");

  // rankx = ranked[0:x.n_elem]  # get the x-ranks
  arma::vec rankx = ranked.head(x.n_elem);

  // u1 = x.n_elem*y.n_elem + (x.n_elem*(x.n_elem+1))/2.0 - np.sum(rankx,
  // axis=0)  # calc U for x
  double u1 = x.n_elem * y.n_elem + (x.n_elem * (x.n_elem + 1)) / 2.0 -
              arma::accu(rankx); // calc U for x;

  double u2 = x.n_elem * y.n_elem - u1; // remainder is U for y

  // T = tiecorrect(ranked)
  double T = tie_correct(ranked);

  // if (T == 0.):
  //     raise ValueError('All numbers are identical in mannwhitneyu')

  double sd =
      std::sqrt(T * x.n_elem * y.n_elem * (x.n_elem + y.n_elem + 1) / 12.0);

  double meanrank = x.n_elem * y.n_elem / 2.0 + 0.5 * use_continuity;

  double bigu{0};
  if (alternative == TestStrategy::TestAlternative::TwoSided)
    bigu = std::max(u1, u2);
  else if (alternative == TestStrategy::TestAlternative::Less)
    bigu = u1;
  else if (alternative == TestStrategy::TestAlternative::Greater)
    bigu = u2;

  double z = (bigu - meanrank) / sd;

  double p{0.};
  normal norm(0, 1);

  if (alternative == TestStrategy::TestAlternative::TwoSided) {
    p = 2 * cdf(complement(norm, fabs(z)));
    if (p < alpha)
      sig = true;
    else
      sig = false;
  } else {
    p = cdf(norm, z);
    if (p < alpha)
      sig = true;
    else
      sig = false;
  }

  double u = u2;

  int eff_side = std::copysign(1.0, Sm2 - Sm1);

  return {u, p, eff_side, sig};
}

double tie_correct(const arma::vec &rankvals) {

  arma::vec arr = arma::sort(rankvals);

  arma::uvec vindx = arma::join_cols(
      arma::uvec({1}), arr.tail(arr.n_elem - 1) != arr.head(arr.n_elem - 1));

  arma::uvec vvindx = arma::join_cols(vindx, arma::uvec({1}));

  arma::uvec indx = nonzeros_index(vvindx);

  arma::uvec cnt = arma::diff(indx);

  int size = arr.n_elem;

  if (size < 2) {
    return 1.0;
  }

  return 1.0 - arma::accu(arma::pow(cnt, 3) - cnt) / (std::pow(size, 3) - size);
}

arma::vec rankdata(const arma::Row<double> &arr,
                   const std::string method = "average") {

  // if method not in ('average', 'min', 'max', 'dense', 'ordinal'):
  //     raise ValueError('unknown method "{0}"'.format(method))

  // arr = np.ravel(np.asarray(a))
  // > This is always true, for now

  // algo = 'mergesort' if method == 'ordinal' else 'quicksort'
  arma::uvec sorter = arma::stable_sort_index(arr);

  // inv = np.empty(sorter.size, dtype=np.intp)
  // inv[sorter] = np.arange(sorter.size, dtype=np.intp)

  arma::uvec inv(sorter.n_elem);

  inv.elem(sorter) = arma::regspace<arma::uvec>(0, sorter.n_elem - 1);

  //        if (method == "ordinal")
  //            return inv + 1;

  arma::vec arr_sorted(arr.n_elem);
  arr_sorted = arr(sorter);

  // obs = np.r_[True, arr[1:] != arr[:-1]]
  arma::uvec obs = arma::join_cols(arma::uvec({1}),
                                   arr_sorted.tail(arr_sorted.n_elem - 1) !=
                                       arr_sorted.head(arr_sorted.n_elem - 1));

  // dense = obs.cumsum()[inv]
  arma::uvec dense = arma::cumsum(obs); //.elem(inv);
  arma::uvec dense_sorted(dense.n_elem);
  dense_sorted = dense(inv);

  if (method == "dense")
    return arma::conv_to<arma::vec>::from(dense_sorted);

  // cumulative counts of each unique value
  // count = [np.r_[np.nonzero(obs)[0], len(obs)]]
  arma::uvec count =
      arma::join_cols(nonzeros_index(obs), arma::uvec({obs.n_elem}));

  //        if (method == "max")
  //            return count(dense_sorted);
  //
  //        if (method == "min")
  //            return count(dense_sorted - 1) + 1;

  // average method
  return .5 * arma::conv_to<arma::vec>::from(
                  (count(dense_sorted) + count(dense_sorted - 1) + 1));
}

} // namespace sam
