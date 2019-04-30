//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_TESTSTRATEGIES_H
#define SAMPP_TESTSTRATEGIES_H

// #include <Experiment.h>
#include <memory>
#include <iostream>
#include <string>
#include <map>
#include <string>
#include <armadillo>

#include "nlohmann/json.hpp"

namespace sam {

    using json = nlohmann::json;
    
    /**
     * @brief      Specifying the significant testing method
     */
    enum class TestMethod {
        TTest,           ///< T-test
        FTest            ///< F-test
    };

    
    enum class TestSide {
        Less,
        Greater,
        TwoSide
    };

    struct TestResult {
        double statistic = 0;
        double pvalue = 0;
        int side = 0;
        bool sig = 0;

        TestResult() = default;
        
        TestResult(double statistic, double pvalue, int side, double sig) :
        statistic(statistic), pvalue(pvalue), side(side), sig(sig) {};
        
        
    };


    class Experiment;
    class ExperimentSetup;

    /**
     @brief Abstract class for defining test strategies.
     
     Statistical test strategies will investigate if there is a meaningful difference
     between means of two samples. Every test strategy should provide a `run()` method.
     The `run()` method will accept a pointer to the experiment and update necessary
     variables, e.g., _statistics_ & _p-value_.
     */
    class TestStrategy {

    public:
        
        static TestStrategy* build(json &config);
        
        virtual ~TestStrategy() = 0;
        
        virtual void run(Experiment* experiment) = 0;

    };

    /**
     @brief Declration of t-test.
     
     The `run()` method will check the significance of the difference between two groups.
     In the current setup, every `experiment->means` is considered an effect size between
     a treatment group and a control group with the mean of zero. Therefore, computing the
     t-statistics and computing the p-value would be sufficient. This is technically an
     implementation of [one sample t-test](https://en.wikipedia.org/wiki/Student%27s_t-test#One-sample_t-test).
     
     */
    class TTest : public TestStrategy {
        
    private:
        TestSide side;
        double alpha;
        
    public:
        TTest() = default;
        
        TTest(TestSide side = TestSide::TwoSide,
              double alpha = 0.05) :
            side(side), alpha(alpha)
        {};
        
        void run(Experiment* experiment);
        
    };


    // Stats Utility

    double single_sample_find_df(double M, double Sm, double Sd, double alpha, TestSide side);

    std::pair<double, double>
    confidence_limits_on_mean(double Sm, double Sd, unsigned Sn, double alpha, TestSide side);

    TestResult
    t_test(arma::Row<double> d1, arma::Row<double> d2, double alpha, TestSide side);

    TestResult
    t_test(double Sm1, double Sd1, double Sn1, double Sm2, double Sd2, double Sn2, double alpha, TestSide side, bool equal_var);

    TestResult
    single_sample_t_test(double M, double Sm, double Sd, unsigned Sn, double alpha, TestSide side);

    TestResult
    two_samples_t_test_equal_sd(double Sm1, double Sd1, unsigned Sn1, double Sm2, double Sd2, unsigned Sn2, double alpha, TestSide side);

    TestResult
    two_samples_t_test_unequal_sd(double Sm1, double Sd1, unsigned Sn1, double Sm2, double Sd2, unsigned Sn2, double alpha, TestSide side);

}

#endif //SAMPP_TESTSTRATEGIES_H

