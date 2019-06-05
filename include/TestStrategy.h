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

#include "sam.h"
#include "Utilities.h"

#include "nlohmann/json.hpp"
#include "utils/magic_enum.hpp"

namespace sam {

    using json = nlohmann::json;


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
        
        struct TestResult {
            double statistic = 0;
            double pvalue = 0;
            int side = 0;
            bool sig = 0;
            
            TestResult() = default;
            
            TestResult(double statistic, double pvalue, int side, double sig) :
            statistic(statistic), pvalue(pvalue), side(side), sig(sig) {};

            friend std::ostream &operator<<(std::ostream &os, const TestResult &result) {
                os << "statistic: " << result.statistic << " pvalue: " << result.pvalue << " side: " << result.side
                   << " sig: " << result.sig;
                return os;
            }
        };
        
        /**
         * Specifying the significant testing method
         */
        enum class TestMethod {
            TTest,           ///< T-test
            FTest            ///< F-test
        };
        
        /**
         Specify the side of the test
         */
        enum class TestSide {
            Less,
            Greater,
            TwoSided
        };
        
        
        /**
         Contains Test Strategy parameters.
         
         @note I'm still experimenting with this while I'm trying to simplify
         the construction process.
         */
        struct TestStrategyParameters {
            TestMethod name;
            TestSide side = TestSide::TwoSided;
            double alpha = 0.05;
        };
//
//        TestStrategyParameters params;
        
        static std::unique_ptr<TestStrategy> build(json &config);
        
        static std::shared_ptr<TestStrategy> build(ExperimentSetup &setup);
        
        static std::shared_ptr<TestStrategy> build(const TestStrategyParameters &params);
        
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
        
    public:
        
        struct Parameters {
            TestMethod name = TestMethod::TTest;
            TestSide side = TestSide::TwoSided;
            double alpha;
        };
        
        Parameters params;
        
        TTest(const Parameters &p) : params{p} {};
        
        // Cleanup
        TTest(TestStrategyParameters tsp) {
//            params = tsp;
        };
        
        void run(Experiment* experiment);
        
    };
    
    
        // JSON Parser for ImpatientDecisionStrategy::Parameters
        inline
        void to_json(json& j, const TTest::Parameters& p) {
            j = json{
                {"name", magic_enum::enum_name<TestStrategy::TestMethod>(p.name)},
                {"side", magic_enum::enum_name<TestStrategy::TestSide>(p.side)},
                {"alpha", p.alpha}
            };
        }
    
        inline
        void from_json(const json& j, TTest::Parameters& p) {
            
            // Using a helper template function to handle the optional and throw if necessary.
            p.name = get_enum_value_from_json<TestStrategy::TestMethod>("name", j);
            
            p.side = get_enum_value_from_json<TestStrategy::TestSide>("side", j);
            
            j.at("alpha").get_to(p.alpha);
        }


    // Stats Utility

    double single_sample_find_df(double M, double Sm, double Sd, double alpha, TestStrategy::TestSide side);

    std::pair<double, double>
    confidence_limits_on_mean(double Sm, double Sd, unsigned Sn, double alpha, TestStrategy::TestSide side);

    TestStrategy::TestResult
    t_test(const arma::Row<double> &d1, const arma::Row<double> &d2, double alpha, TestStrategy::TestSide side);

    TestStrategy::TestResult
    t_test(double Sm1, double Sd1, double Sn1, double Sm2, double Sd2, double Sn2, double alpha, TestStrategy::TestSide side, bool equal_var);

    TestStrategy::TestResult
    single_sample_t_test(double M, double Sm, double Sd, unsigned Sn, double alpha, TestStrategy::TestSide side);

    TestStrategy::TestResult
    two_samples_t_test_equal_sd(double Sm1, double Sd1, unsigned Sn1, double Sm2, double Sd2, unsigned Sn2, double alpha, TestStrategy::TestSide side);

    TestStrategy::TestResult
    two_samples_t_test_unequal_sd(double Sm1, double Sd1, unsigned Sn1, double Sm2, double Sd2, unsigned Sn2, double alpha, TestStrategy::TestSide side);

}

#endif //SAMPP_TESTSTRATEGIES_H

