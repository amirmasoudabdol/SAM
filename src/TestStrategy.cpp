//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <spdlog/spdlog.h>

#include <boost/math/distributions/fisher_f.hpp>
#include <boost/math/distributions/normal.hpp>
#include <boost/math/distributions/students_t.hpp>

#include "TestStrategy.h"

using namespace sam;

using boost::math::students_t;

TestStrategy::~TestStrategy(){
    // Pure deconstructor
};

std::unique_ptr<TestStrategy> TestStrategy::build(json &test_strategy_config) {

  if (test_strategy_config["name"] == "TTest") {

    auto params = test_strategy_config.get<TTest::Parameters>();
    return std::make_unique<TTest>(params);

  } else if (test_strategy_config["name"] == "YuenTest") {

    auto params = test_strategy_config.get<YuenTest::Parameters>();
    return std::make_unique<YuenTest>(params);

  } else if (test_strategy_config["name"] == "WilcoxonTest") {
    auto params = test_strategy_config.get<WilcoxonTest::Parameters>();
    return std::make_unique<WilcoxonTest>(params);
  } else {
    throw std::invalid_argument("Unknown Test Strategy.");
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
/// @note       Obtained from [Boost Library
///             Example](https://www.boost.org/doc/libs/1_69_0/libs/math/doc/html/math_toolkit/stat_tut/weg/st_eg/paired_st.html).
///
/// @param      Sm    Sample Mean.
/// @param      Sd    Sample Standard Deviation.
/// @param      Sn    Sample Size.
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
/// @param      M     True Mean.
/// @param      Sm    Sample Mean.
/// @param      Sd    Sample Standard Deviation.
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

  auto ibot = floor(trim * x.n_elem) + 1;
  auto itop = x.n_elem - ibot + 1;

  double xbot{y.at(ibot - 1)};
  double xtop{y.at(itop - 1)};

  return arma::clamp(x, xbot, xtop);
}

// TODO: this can be an extention to arma, something like I did for
// nlohmann::json I should basically put it into arma's namespace
double trim_mean(const arma::Row<double> &x, double trim) {
  arma::rowvec y{arma::sort(x)};

  auto ibot = floor(trim * x.n_elem) + 1;
  auto itop = x.n_elem - ibot + 1;

  return arma::mean(y.subvec(ibot - 1, itop - 1));
}



double tie_correct(const arma::vec &rankvals) {

  arma::vec arr = arma::sort(rankvals);

  arma::uvec vindx = arma::join_cols(
      arma::uvec({1}), arr.tail(arr.n_elem - 1) != arr.head(arr.n_elem - 1));

  arma::uvec vvindx = arma::join_cols(vindx, arma::uvec({1}));

  arma::uvec indx = nonzeros_index(vvindx);

  arma::uvec cnt = arma::diff(indx);

  auto size = arr.n_elem;

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
