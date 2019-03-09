//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <iostream>
#include <ostream>
#include <string>
#include <algorithm>
#include <numeric>

#include "Utilities.h"
#include "gsl/gsl_rng.h"
#include "gsl/gsl_randist.h"
#include "gsl/gsl_statistics.h"


std::pair<double, double>
equal_var_ttest_denom(double v1, int n1, double v2, int n2) {

    double df = n1 + n2 - 2.;
    double svar = ((n1 - 1) * v1 + (n2 - 1) * v2) / df;
    double denom = sqrt(svar * (1.0 / n1 + 1.0 / n2));
    return std::make_pair(df, denom);
}

double ttest_finish(double t, double df) {
    return gsl_ran_tdist_pdf(t, df) * 2;
}

std::pair<double, double>
oneSampleTTest(double mean1, double sd1, int nobs1, double mean2, double sd2, int nobs2, bool equal_var) {

    std::pair<double, double> df_denom;
    if (equal_var) {
        df_denom = equal_var_ttest_denom(sd1 * sd1, nobs1, sd2 * sd2, nobs2);
    }

    double d = mean1 - mean2;
    double t = d / df_denom.second;
    double prob = ttest_finish(t, df_denom.first);

    return std::make_pair(t, prob);
}

