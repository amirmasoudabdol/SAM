//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <cmath>

//#include <utils/magic_enum.hpp>
#include <boost/math/distributions/students_t.hpp>
#include <boost/math/distributions/fisher_f.hpp>
#include <boost/math/distributions/normal.hpp>

#include "Experiment.h"
#include "TestStrategy.h"

using namespace sam;
using boost::math::students_t;

TestStrategy::~TestStrategy() {
    // Pure deconstructor
};


std::unique_ptr<TestStrategy> TestStrategy::build(json &test_strategy_config){
    
    if (test_strategy_config["_name"] == "TTest"){
        
        auto params = test_strategy_config.get<TTest::Parameters>();
        return std::make_unique<TTest>(params);
        
    } else if (test_strategy_config["_name"] == "YuenTTest") {
        
        auto params = test_strategy_config.get<YuenTTest::Parameters>();
        return std::make_unique<YuenTTest>(params);
        
    } else if (test_strategy_config["_name"] == "MannWhitneyWilcoxon"){
        auto params = test_strategy_config.get<MannWhitneyWilcoxon::Parameters>();
        return std::make_unique<MannWhitneyWilcoxon>(params);
    }
    else{
        throw std::invalid_argument("Unknown Test Strategy.");
    }
    
}

void TTest::run(Experiment* experiment) {
    
    static TestStrategy::TestResult res;
    
    // The first group is always the control group
    for (int i{experiment->setup.nd()}, d{0};
         i < experiment->measurements.size();   // technically experiment->setup.ng();
         ++i, d%=experiment->setup.nd()) {
        
        // This is not perfect, basically I need to check the population `vars`
//        if ((isgreater(experiment->stddev[d], experiment->stddev[i]) or isless(experiment->stddev[d], experiment->stddev[i]))){
//
//            res = two_samples_t_test_unequal_sd(experiment->means[d],
//                                               experiment->stddev[d],
//                                               experiment->measurements[d].size(),
//                                               experiment->means[i],
//                                               experiment->stddev[i],
//                                               experiment->measurements[i].size(),
//                                               params.alpha, params.side);
//        }else {
            // EQUAL SD
            res = two_samples_t_test_equal_sd(experiment->means[d],
                                               experiment->stddev[d],
                                               experiment->measurements[d].size(),
                                               experiment->means[i],
                                               experiment->stddev[i],
                                               experiment->measurements[i].size(),
                                               params.alpha, params.side);
//        }
        
        
        experiment->statistics[i] = res.statistic;
        experiment->pvalues[i] = res.pvalue;
        experiment->sigs[i] = res.sig;
    }
}

void YuenTTest::run(Experiment* experiment) {
    
    // The first group is always the control group
    
    static TestStrategy::TestResult res;
    
    for (int i{experiment->setup.nd()}, d{0};
         i < experiment->measurements.size();
         ++i, d%=experiment->setup.nd()) {
        
//        if (experiment->measurements[d].size() == experiment->measurements[i].size()){
//
//            res = yuen_t_test_paired(experiment->measurements[d],
//                                       experiment->measurements[i],
//                                       params.alpha,
//                                       params.side,
//                                       params.trim,
//                                       0);
//        }else{
            res = yuen_t_test_two_samples(experiment->measurements[d],
                                        experiment->measurements[i],
                                        params.alpha,
                                        params.side,
                                        params.trim,
                                        0);
//        }
                
        experiment->statistics[i] = res.statistic;
        experiment->pvalues[i] = res.pvalue;
        experiment->sigs[i] = res.sig;
    }
}


void MannWhitneyWilcoxon::run(Experiment* experiment) {
    
    // The first group is always the control group
    
    static TestStrategy::TestResult res;
    
    for (int i{experiment->setup.nd()}, d{0};
         i < experiment->measurements.size();
         ++i, d%=experiment->setup.nd()) {
        
//        if (experiment->measurements[d].size() == experiment->measurements[i].size()){
//
//            res = yuen_t_test_paired(experiment->measurements[d],
//                                       experiment->measurements[i],
//                                       params.alpha,
//                                       params.side,
//                                       params.trim,
//                                       0);
//        }else{
            res = mann_whitney_wilcoxon_u_test(experiment->measurements[d],
                                                experiment->measurements[i],
                                                params.alpha,
                                                params.side);
//        }
                
        experiment->statistics[i] = res.statistic;
        experiment->pvalues[i] = res.pvalue;
        experiment->sigs[i] = res.sig;
    }
}




namespace sam {

    /**
     Calculate confidence intervals for the mean.
     For example if we set the confidence limit to
     0.95, we know that if we repeat the sampling
     100 times, then we expect that the true mean
     will be between out limits on 95 occations.
     Note: this is not the same as saying a 95%
     confidence interval means that there is a 95%
     probability that the interval contains the true mean.
     The interval computed from a given sample either
     contains the true mean or it does not.
     
     @note Obtained from [Boost Library Example](https://www.boost.org/doc/libs/1_69_0/libs/math/doc/html/math_toolkit/stat_tut/weg/st_eg/paired_st.html).
     
     @param Sm Sample Mean.
     @param Sd Sample Standard Deviation.
     @param Sn Sample Size.
     */
    std::pair<double, double>
    confidence_limits_on_mean(double Sm, double Sd, unsigned Sn, double alpha, TestStrategy::TestSide side)
    {

        using namespace sam;

        students_t dist(Sn - 1);
        
        // calculate T
        double T = quantile(complement(dist, alpha / 2));   // TODO: Implement the side!./sa
        
        // Calculate width of interval (one sided):
        double w = T * Sd / sqrt(double(Sn));
        
        // Calculate and return the interval
        return std::make_pair(Sm - w, Sm + w);
        
    }


    /**
     Caculate the degress of freedom to achieve a significance result with the given alpha
     
     @param M True Mean.
     @param Sm Sample Mean.
     @param Sd Sample Standard Deviation.
     */
    double single_sample_find_df(double M,
                                 double Sm, double Sd, double alpha, TestStrategy::TestSide side)
    {
        using namespace sam;
        using boost::math::students_t;
        
        // calculate df for one-sided or two-sided test:
        double df = students_t::find_degrees_of_freedom(fabs(M - Sm),
                                                        (side == TestStrategy::TestSide::Greater) ? alpha : alpha / 2. ,
                                                        alpha,
                                                        Sd);
        
        // convert to sample size, always one more than the degrees of freedom:
        return ceil(df) + 1;
    }

    TestStrategy::TestResult t_test(const arma::Row<double> &dt1,
                                    const arma::Row<double> &dt2,
                                    double alpha, TestStrategy::TestSide side){
        return t_test(arma::mean(dt1), arma::stddev(dt1), dt1.size(),
                      arma::mean(dt2), arma::stddev(dt2), dt2.size(),
                      alpha, side, true);
    }


    TestStrategy::TestResult
    t_test(double Sm1, double Sd1, double Sn1,
           double Sm2, double Sd2, double Sn2,
           double alpha, TestStrategy::TestSide side, bool equal_var = false){

        using namespace sam;

        if (Sm1 == 0.){
            return single_sample_t_test(Sm1,
                                        Sm2, Sd2, Sn2,
                                        alpha, side);
        }
        
        if (equal_var) {
            return two_samples_t_test_equal_sd(Sm1, Sd1, Sn1,
                                               Sm2, Sd2, Sn2,
                                               alpha, side);
        }else{
            return two_samples_t_test_unequal_sd(Sm1, Sd1, Sn1,
                                                 Sm2, Sd2, Sn2,
                                                 alpha, side);
        }
    }


    /**
     A Students t test applied to a single set of data.
     We are testing the null hypothesis that the true
     mean of the sample is M, and that any variation is down
     to chance.  We can also test the alternative hypothesis
     that any difference is not down to chance
     
     @note Obtained from [Boost Library Example](https://www.boost.org/doc/libs/1_69_0/libs/math/doc/html/math_toolkit/stat_tut/weg/st_eg/paired_st.html).
     
     @param M True Mean.
     @param Sm Sample Mean.
     @param Sd Sample Standard Deviation.
     @param Sn Sample Size.
     @param alpha Significance Level.
     @return TestStrategy::TestResult
     */
    TestStrategy::TestResult
    single_sample_t_test(double M,
                         double Sm, double Sd, unsigned Sn,
                         double alpha, TestStrategy::TestSide side)
    {
        
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
        
        if (side == TestStrategy::TestSide::TwoSided){
            // Mean != M
            p = 2 * cdf(complement(dist, fabs(t_stat)));
            if(p < alpha){
                // Alternative "NOT REJECTED"
                sig = true;
            }
            else{
                // Alternative "REJECTED"
                sig = false;
            }
        }

        if (side == TestStrategy::TestSide::Greater){
            // Mean  > M
            p = cdf(dist, t_stat);
            if(p > alpha){
                // Alternative "NOT REJECTED"
                sig = true;
            }
            else{
                // Alternative "REJECTED"
                sig = false;
            }
        }
        
        if (side == TestStrategy::TestSide::Less){
            // Mean  < M
            p = cdf(complement(dist, t_stat));
            if(p > alpha){
                // Alternative "NOT REJECTED"
                sig = true;
            }
            else{
                // Alternative "REJECTED"
                sig = false;
            }
        }
        
        int mside = std::copysign(1.0, M - Sm);

        return {t_stat, p, mside, sig};
    }


    /**
     A Students t test applied to two sets of data.
     We are testing the null hypothesis that the two
     samples have the same mean and that any difference
     if due to chance.
     
     @note Obtained from [Boost Library Example](https://www.boost.org/doc/libs/1_69_0/libs/math/doc/html/math_toolkit/stat_tut/weg/st_eg/paired_st.html).
     
     @param Sm1 Sample Mean 1.
     @param Sd1 Sample Standard Deviation 1.
     @param Sn1 Sample Size 1.
     @param Sm2 Sample Mean 2.
     @param Sd2 Sample Standard Deviation 2.
     @param Sn2 Sample Size 2.
     @param alpha Significance Level.
     @return TestStrategy::TestResult
     */
    TestStrategy::TestResult two_samples_t_test_equal_sd(double Sm1, double Sd1, unsigned Sn1, double Sm2, double Sd2, unsigned Sn2, double alpha, TestStrategy::TestSide side)
    {
        
        bool sig = false;
        
        // Degrees of freedom:
        double df = Sn1 + Sn2 - 2;
        
        // Pooled variance and hence standard deviation:
        double sp = sqrt(((Sn1-1) * Sd1 * Sd1 + (Sn2-1) * Sd2 * Sd2) / df);
        
        // NOTE: I had to do this, I don't want my simulations fail.
        // While this is not perfect, it allows me to handle an edge case
        // SAM should not throw and should continue working.
        if (!(isgreater(sp, 0) or isless(sp, 0))){
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
        
        if (side == TestStrategy::TestSide::TwoSided){
            // Sample 1 Mean != Sample 2 Mean
            p = 2 * cdf(complement(dist, fabs(t_stat)));
            if(p < alpha){
                // Alternative "NOT REJECTED"
                sig = true;
            }else{
                // Alternative "REJECTED"
                sig = false;
            }
        }
        
        if (side == TestStrategy::TestSide::Greater){
            // Sample 1 Mean <  Sample 2 Mean
            p = cdf(dist, t_stat);
            if(p< alpha){
                // Alternative "NOT REJECTED"
                sig = true;
            }else{
                // Alternative "REJECTED"
                sig = false;
            }
        }
        
        if (side == TestStrategy::TestSide::Less){
            
            // Sample 1 Mean >  Sample 2 Mean
            p = cdf(complement(dist, t_stat));
            if(p< alpha){
                // Alternative "NOT REJECTED"
                sig = true;
            }else{
                sig = false;
                // Alternative "REJECTED"
            }
        }
        
        int mside = std::copysign(1.0, Sm1 - Sm2);
        
        return {t_stat, p, mside, sig};
    }


    /**
     A Students t test applied to two sets of data with _unequal_ variance.
     We are testing the null hypothesis that the two
     samples have the same mean and
     that any difference is due to chance.
     
     @note Obtained from [Boost Library Example](https://www.boost.org/doc/libs/1_69_0/libs/math/doc/html/math_toolkit/stat_tut/weg/st_eg/paired_st.html).
     
     @param Sm1 Sample Mean 1.
     @param Sd1 Sample Standard Deviation 1.
     @param Sn1 Sample Size 1.
     @param Sm2 Sample Mean 2.
     @param Sd2 Sample Standard Deviation 2.
     @param Sn2 Sample Size 2.
     @param alpha Significance Level.
     @return TestStrategy::TestResult
     */
    TestStrategy::TestResult two_samples_t_test_unequal_sd(double Sm1, double Sd1, unsigned Sn1, double Sm2, double Sd2, unsigned Sn2, double alpha, TestStrategy::TestSide side)
    {
        
        
        bool sig = false;
        
        
        // Degrees of freedom:
        double df = Sd1 * Sd1 / Sn1 + Sd2 * Sd2 / Sn2;
        df *= df;
        double t1 = Sd1 * Sd1 / Sn1;
        t1 *= t1;
        t1 /=  (Sn1 - 1);
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
        
        if (side == TestStrategy::TestSide::TwoSided){
            // Sample 1 Mean != Sample 2 Mean
            p = 2 * cdf(complement(dist, fabs(t_stat)));
            if(p < alpha){
                // Alternative "NOT REJECTED"
                sig = true;
            }else{
                // Alternative "REJECTED"
                sig = false;
            }
        }
        
        if (side == TestStrategy::TestSide::Greater){
            // Sample 1 Mean <  Sample 2 Mean
            p = cdf(dist, t_stat);
            if(p< alpha){
                // Alternative "NOT REJECTED"
                sig = true;
            }else{
                // Alternative "REJECTED"
                sig = false;
            }
        }
        
        if (side == TestStrategy::TestSide::Less){
            // Sample 1 Mean >  Sample 2 Mean
            p = cdf(complement(dist, t_stat));
            if(p< alpha){
                // Alternative "NOT REJECTED"
                sig = true;
            }else{
                sig = false;
                // Alternative "REJECTED"
            }
        }
        
        int mside = std::copysign(1.0, Sm1 - Sm2);
        
        return {t_stat, p, mside, sig};
    }

    TestStrategy::TestResult
    f_test(
        double sd1,     // Sample 1 std deviation
        double sd2,     // Sample 2 std deviation
        double N1,      // Sample 1 size
        double N2,      // Sample 2 size
        double alpha)  // Significance level
    {

        using boost::math::fisher_f;
        
        bool sig = false;

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
        if((ucv2 < f_stats) || (lcv2 > f_stats))
            // Alternative "NOT REJECTED"
            sig = true;
        else
            // Alternative "REJECTED"
            sig = false;

        if(lcv > f_stats)
            // Alternative "NOT REJECTED"
            sig = true;
        else
            // Alternative "REJECTED"
            sig = false;

        if(ucv < f_stats)
            // Alternative "NOT REJECTED"
            sig = true;
        else
            // Alternative "REJECTED"
            sig = false;

//        int mside = std::copysign(1.0, Sm1 - Sm2);
        
        return {f_stats, p, 1, sig};
    }


    TestStrategy::TestResult yuen_t_test_paired(
                                         const arma::Row<double> &x,
                                         const arma::Row<double> &y,
                                         double alpha,
                                         const TestStrategy::TestSide side,
                                         double trim = 0.2,
                                         double mu = 0){
        // Do some check whether it's possible to run the test
        
        double Sm1 = arma::mean(x);
        double Sm2 = arma::mean(y);
        
        bool sig {false};
        
        int h1 = x.n_elem - 2 * static_cast<int>(floor(trim * x.n_elem));
        
        double q1 = (x.n_elem - 1) * win_var(x, trim);

        double q2 = (y.n_elem - 1) * win_var(y, trim);
        
        double q3 = (x.n_elem - 1) * std::get<1>(win_cor_cov(x, y, trim));
        
        unsigned df = h1 - 1;

        double se = sqrt( (q1 + q2 - 2 * q3) / (h1 * (h1 - 1)) );
        
        double dif = trim_mean(x, trim) - trim_mean(y, trim);
        
        double t_stat = (dif - mu) / se;
        
        students_t dist(df);
        double p = 0;
        
        if (side == TestStrategy::TestSide::TwoSided){
            // Mean != M
//            p = 2 * (1 - cdf(dist, fabs(t_stat)));
            p = 2 * cdf(complement(dist, fabs(t_stat)));
            if(p < alpha / 2){
                // Alternative "NOT REJECTED"
                sig = true;
            }
            else{
                // Alternative "REJECTED"
                sig = false;
            }
        }
        
        if (side == TestStrategy::TestSide::Greater){
            // Mean  > M
//            p = 1 - cdf(dist, t_stat);
            p = cdf(complement(dist, t_stat));
            if(p > alpha){
                // Alternative "NOT REJECTED"
                sig = true;
            }
            else{
                // Alternative "REJECTED"
                sig = false;
            }
        }
        
        if (side == TestStrategy::TestSide::Less){
            // Mean  < M
//            p = cdf(dist, t_stat);
            p = cdf(dist, t_stat);
            if(p > alpha){
                // Alternative "NOT REJECTED"
                sig = true;
            }
            else{
                // Alternative "REJECTED"
                sig = false;
            }
        }
        
        int mside = std::copysign(1.0, Sm1 - Sm2);
        
        return {t_stat, p, mside, sig};
    }


    TestStrategy::TestResult yuen_t_test_two_samples(
                                                  const arma::Row<double> &x,
                                                  const arma::Row<double> &y,
                                                  double alpha,
                                                  const TestStrategy::TestSide side,
                                                  double trim,
                                                  double mu){
        
        double Sm1 = arma::mean(x);
        double Sm2 = arma::mean(y);
        
        bool sig {false};

        int h1 = x.n_elem - 2 * floor(trim * x.n_elem);
        int h2 = y.n_elem - 2 * floor(trim * y.n_elem);

        double d1 = (x.n_elem - 1) * win_var(x, trim) / (h1 * (h1 - 1));
        double d2 = (y.n_elem - 1) * win_var(y, trim) / (h2 * (h2 - 1));

        unsigned df = pow(d1+d2, 2) / (pow(d1, 2) / (h1-1) + pow(d2, 2) / (h2-1));

        double se = sqrt(d1+d2);

        double dif = trim_mean(x, trim) - trim_mean(y, trim);

        double t_stat = (dif - mu) / se;

        students_t dist(df);
        double p;
        
        if (side == TestStrategy::TestSide::TwoSided){
            // Sample 1 Mean != Sample 2 Mean
            p = 2 * cdf(complement(dist, fabs(t_stat)));
            if(p < alpha / 2){
                // Alternative "NOT REJECTED"
                sig = true;
            }else{
                // Alternative "REJECTED"
                sig = false;
            }
        }
        
        if (side == TestStrategy::TestSide::Greater){
            // Sample 1 Mean <  Sample 2 Mean
            p = cdf(dist, t_stat);
            if(p< alpha){
                // Alternative "NOT REJECTED"
                sig = true;
            }else{
                // Alternative "REJECTED"
                sig = false;
            }
        }
        
        if (side == TestStrategy::TestSide::Less){
            // Sample 1 Mean >  Sample 2 Mean
            p = cdf(complement(dist, t_stat));
            if(p< alpha){
                // Alternative "NOT REJECTED"
                sig = true;
            }else{
                sig = false;
                // Alternative "REJECTED"
            }
        }

        int mside = std::copysign(1.0, Sm1 - Sm2);
        
        return {t_stat, p, mside, sig};

    }


    double win_var(const arma::Row<double> &x,
                   const double trim) {
        return arma::var(win_val(x, trim));
    }

    std::pair<double, double>
    win_cor_cov(const arma::Row<double> &x,
                   const arma::Row<double> &y,
                   const double trim){
                
        arma::rowvec xvec { win_val(x, trim) };
        arma::rowvec yvec { win_val(y, trim) };

        arma::mat wcor { arma::cor(xvec, yvec) };
        double vwcor { wcor.at(0, 0) };

        arma::mat wcov { arma::cov(xvec, yvec) };
        double vwcov { wcov.at(0, 0)};

        return std::make_pair(vwcor, vwcov);
        
    }

    arma::Row<double> win_val(const arma::Row<double> &x,
                              double trim){

        arma::rowvec y { arma::sort(x) };
        
        int ibot = floor(trim * x.n_elem) + 1;
        int itop = x.n_elem - ibot + 1;
        
        double xbot { y.at(ibot - 1) };
        double xtop { y.at(itop - 1) };

        return arma::clamp(x, xbot, xtop);
    }


    // TODO: this can be an extention to arma, something like I did for nlohmann::json
    // I should basically put it into arma's namespace
    double trim_mean(const arma::Row<double> &x,
                     double trim){
        arma::rowvec y { arma::sort(x) };
        
        int ibot = floor(trim * x.n_elem) + 1;
        int itop = x.n_elem - ibot + 1;
        
        return arma::mean(y.subvec(ibot - 1, itop - 1));
    }

    TestStrategy::TestResult mann_whitney_wilcoxon_u_test(const arma::Row<double> &x,
                                                          const arma::Row<double> &y,
                                                          double alpha,
                                                          const TestStrategy::TestSide side){
        
        arma::rowvec z = arma::join_rows(x, y);
        arma::uvec ranks = arma::sort_index(z);
    
        double sum_rank_x = arma::accu(ranks.subvec(0, x.n_elem));
        
        double u1 = sum_rank_x - (x.n_elem * (x.n_elem + 1)) / 2;
        
        double u2 = x.n_elem * y.n_elem - u1;

        double u_max = std::max(u1, u2);
        
        double u_min = x.n_elem * y.n_elem - u_max;
        
        long nx_ny = x.n_elem * y.n_elem;

        double mu_u = nx_ny / 2.0;
        double var_u = nx_ny * (x.n_elem + y.n_elem + 1) / 12.0;

        double z_val = (u_min - mu_u) / sqrt(var_u);

        normal standard_normal(0, 1);

        double pval = 2 * cdf(standard_normal,z_val);
        
        return TestStrategy::TestResult(z_val, pval, 1, 0);
    }

}
