//
// Created by Amir Masoud Abdol on 2020-04-11
//

#include "TestStrategy.h"

#include <boost/math/distributions/fisher_f.hpp>

using namespace sam;
using boost::math::fisher_f;

void FTest::run(Experiment *experiment) {

  static ResultType res;

  // The first group is always the control group
  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {

    res =
        f_test((*experiment)[d].stddev_, (*experiment)[d].nobs_,
               (*experiment)[i].stddev_, (*experiment)[i].nobs_, params.alpha);

    (*experiment)[i].stats_ = res.fstat;
    (*experiment)[i].pvalue_ = res.pvalue;
    (*experiment)[i].sig_ = res.sig;
  }
}

FTest::ResultType FTest::f_test(float Sd1,   // Sample 1 std deviation
                                unsigned Sn1, // Sample 1 size
                                float Sd2,   // Sample 2 std deviation
                                unsigned Sn2, // Sample 2 size
                                float alpha) // Significance level
{

  bool sig{false};

  // F-statistic:
  float f_stats = (Sd1 / Sd2);

  //
  // Finally define our distribution, and get the probability:
  //
  fisher_f dist(Sn1 - 1, Sn2 - 1);
  float p = cdf(dist, f_stats);

  float ucv = quantile(complement(dist, alpha));
  float ucv2 = quantile(complement(dist, alpha / 2));
  float lcv = quantile(dist, alpha);
  float lcv2 = quantile(dist, alpha / 2);

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

  return {.fstat = f_stats, .df1 = Sn1 - 1, .df2 = Sn2 - 1, .pvalue = p, .sig = sig};
}
