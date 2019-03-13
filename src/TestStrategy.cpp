//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <TestStrategy.h>
#include "Utilities.h"
#include <iostream>

#include <boost/math/distributions/students_t.hpp>
#include <iomanip>
#include <cmath>

#include <Experiment.h>

TestStrategy::~TestStrategy() {
    // Pure deconstructor
};

using boost::math::students_t;

const std::map<std::string, TestSide>
stringToTestSide = {
    {"Less", TestSide::Less},
    {"Greater", TestSide::Greater},
    {"Two Side", TestSide::TwoSide}
};

void TTest::run(Experiment* experiment) {
    
    for (int i = 0; i < experiment->setup.ng; ++i) {
        //        experiment->statistics[i] = experiment->means[i] / experiment->ses[i];
        //
        //        experiment->pvalues[i] = gsl_ran_tdist_pdf(experiment->statistics[i], experiment->measurements[i].size() - 1.);
        
        TestResult res = single_sample_t_test(0,
                                              experiment->means[i],
                                              sqrt(experiment->vars[i]),
                                              experiment->measurements[i].size(),
                                              this->_alpha,
                                              this->_side);
        experiment->statistics[i] = res.statistic;
        experiment->pvalues[i] = res.pvalue;
        experiment->sigs[i] = res.sig;
    }
}

TestStrategy *TestStrategy::buildTestStrategy(json &config){
    
    if (config["type"] == "TTest"){
        return new TTest(stringToTestSide.find(config["side"])->second,
                         config["alpha"]);
    }
    
    return new TTest(TestSide::TwoSide, 0.05);
    
}

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
confidence_limits_on_mean(double Sm, double Sd, unsigned Sn, double alpha, TestSide side)
{
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
double single_sample_find_df(double M, double Sm, double Sd, double alpha, TestSide side)
{
    using boost::math::students_t;
    
    // calculate df for one-sided or two-sided test:
    double df = students_t::find_degrees_of_freedom(fabs(M - Sm),
                                                    (side == TestSide::Greater) ? alpha : alpha / 2. ,
                                                    alpha,
                                                    Sd);
    
    // convert to sample size, always one more than the degrees of freedom:
    return ceil(df) + 1;
}

TestResult t_test(arma::Row<double> dt1, arma::Row<double> dt2, double alpha, TestSide side){
    return t_test(arma::mean(dt1), arma::stddev(dt1), dt1.size(),
                  arma::mean(dt2), arma::stddev(dt2), dt2.size(),
                  alpha, side, true);
}


TestResult
t_test(double Sm1, double Sd1, double Sn1, double Sm2, double Sd2, double Sn2, double alpha, TestSide side, bool equal_var = false){
    if (Sm1 == 0.){
        return single_sample_t_test(Sm1, Sm2, Sd2, Sn2, alpha, side);
    }
    
    if (equal_var) {
        return two_samples_t_test_equal_sd(Sm1, Sd1, Sn1, Sm2, Sd2, Sn2, alpha, side);
    }else{
        return two_samples_t_test_unequal_sd(Sm1, Sd1, Sn1, Sm2, Sd2, Sn2, alpha, side);
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
 @return TestResult
 */
TestResult single_sample_t_test(double M, double Sm, double Sd, unsigned Sn, double alpha, TestSide side)
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
    double q = 0;
    
    //
    // Finally print out results of alternative hypothesis:
    //
    
    if (side == TestSide::TwoSide){
        // Mean != M
        q = 2 * cdf(complement(dist, fabs(t_stat)));
        if(q < alpha / 2){
            // Alternative "NOT REJECTED"
            sig = true;
        }
        else{
            // Alternative "REJECTED"
            sig = false;
        }
    }
    
    if (side == TestSide::Less){
        // Mean  < M
        q = cdf(complement(dist, t_stat));
        if(q > alpha){
            // Alternative "NOT REJECTED"
            sig = true;
        }
        else{
            // Alternative "REJECTED"
            sig = false;
        }
    }
    
    if (side == TestSide::Greater){
        // Mean  > M
        q = cdf(dist, t_stat);
        if(q > alpha){
            // Alternative "NOT REJECTED"
            sig = true;
        }
        else{
            // Alternative "REJECTED"
            sig = false;
        }
    }
    
    return TestResult(t_stat, q, 1, sig);
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
 @return TestResult
 */
TestResult two_samples_t_test_equal_sd(double Sm1, double Sd1, unsigned Sn1, double Sm2, double Sd2, unsigned Sn2, double alpha, TestSide side)  
{
    
    bool sig = false;
    
    // Degrees of freedom:
    double df = Sn1 + Sn2 - 2;
    
    // Pooled variance and hence standard deviation:
    double sp = sqrt(((Sn1-1) * Sd1 * Sd1 + (Sn2-1) * Sd2 * Sd2) / df);
    
    // t-statistic:
    double t_stat = (Sm1 - Sm2) / (sp * sqrt(1.0 / Sn1 + 1.0 / Sn2));
    
    //
    // Define our distribution, and get the probability:
    //
    students_t dist(df);
    double q = 0;
    
    //
    // Finally print out results of alternative hypothesis:
    //
    
    if (side == TestSide::TwoSide){
        // Sample 1 Mean != Sample 2 Mean
        q = 2 * cdf(complement(dist, fabs(t_stat)));
        if(q < alpha / 2){
            // Alternative "NOT REJECTED"
            sig = true;
        }else{
            // Alternative "REJECTED"
            sig = false;
        }
    }
    
    if (side == TestSide::Greater){
        // Sample 1 Mean <  Sample 2 Mean
        q = cdf(dist, t_stat);
        if(q< alpha){
            // Alternative "NOT REJECTED"
            sig = true;
        }else{
            // Alternative "REJECTED"
            sig = false;
        }
    }
    
    if (side == TestSide::Less){
        
        // Sample 1 Mean >  Sample 2 Mean
        q = cdf(complement(dist, t_stat));
        if(q< alpha){
            // Alternative "NOT REJECTED"
            sig = true;
        }else{
            sig = false;
            // Alternative "REJECTED"
        }
    }
    
    return TestResult(t_stat, q, 1, sig);
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
 @return TestResult
 */
TestResult two_samples_t_test_unequal_sd(double Sm1, double Sd1, unsigned Sn1, double Sm2, double Sd2, unsigned Sn2, double alpha, TestSide side)
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
    double q = 0;
    
    //
    // Finally print out results of alternative hypothesis:
    //
    
    if (side == TestSide::TwoSide){
        // Sample 1 Mean != Sample 2 Mean
        q = 2 * cdf(complement(dist, fabs(t_stat)));
        if(q < alpha / 2){
            // Alternative "NOT REJECTED"
            sig = true;
        }else{
            // Alternative "REJECTED"
            sig = false;
        }
    }
    
    if (side == TestSide::Greater){
        // Sample 1 Mean <  Sample 2 Mean
        q = cdf(dist, t_stat);
        if(q< alpha){
            // Alternative "NOT REJECTED"
            sig = true;
        }else{
            // Alternative "REJECTED"
            sig = false;
        }
    }
    
    if (side == TestSide::Less){
        // Sample 1 Mean >  Sample 2 Mean
        q = cdf(complement(dist, t_stat));
        if(q< alpha){
            // Alternative "NOT REJECTED"
            sig = true;
        }else{
            sig = false;
            // Alternative "REJECTED"
        }
    }
    
    return TestResult(t_stat, q, 1, sig);
}

