//
// Created by Amir Masoud Abdol on 2020-04-11
//

#include "TestStrategy.h"

using namespace sam;

void WilcoxonTest::run(Experiment *experiment) {

  // The first group is always the control group

  static ResultType res;

  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {

    res = wilcoxon_test((*experiment)[d].measurements(),
                        (*experiment)[i].measurements(),
                        params.use_continuity, params.alpha,
                        params.alternative);

    (*experiment)[i].stats_ = res.wstat;
    (*experiment)[i].pvalue_ = res.pvalue;
    (*experiment)[i].eff_side_ = res.side;
    (*experiment)[i].sig_ = res.sig;
  }
}

WilcoxonTest::ResultType WilcoxonTest::wilcoxon_test(
    const arma::Row<double> &x, const arma::Row<double> &y, double alpha,
    double use_continuity, const TestStrategy::TestAlternative alternative) {

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

  return {.zstat = z, .wstat = u, .pvalue = p, .side = eff_side, .sig = sig};
}
