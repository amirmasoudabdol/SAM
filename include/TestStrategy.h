//
// Created by Amir Masoud Abdol on 2019-01-24.
//

///
/// @defgroup   TestStrategies Test Strategies
/// @brief      List of available Test Strategies
///
/// Description to come!
///

#ifndef SAMPP_TESTSTRATEGIES_H
#define SAMPP_TESTSTRATEGIES_H

#include "Experiment.h"
#include "Distributions.h"
#include <ostream>

namespace sam {

class DependentVariable;
class ExperimentSetup;
class Experiment;

///
/// @brief      Abstract class for defining test strategies.
///
/// Statistical test strategies will investigate if there is a meaningful
/// difference between means of two samples. Every test strategy should provide
/// a `run()` method. The `run()` method will accept a pointer to the experiment
/// and update necessary variables, e.g., _statistics_ & _p-value_.
///
class TestStrategy {

public:
  virtual ~TestStrategy() = 0;

  ///
  /// Specifying the significant testing method
  ///
  enum class TestMethod {
    TTest,       ///< TTest
    FTest,       ///< FTest
    YuenTest,    ///< YuenTest
    WilcoxonTest ///< WilcoxonTest
  };

  ///
  /// Specify the side of the test
  ///
  enum class TestAlternative { Less, Greater, TwoSided };

  static std::unique_ptr<TestStrategy> build(json &test_strategy_config);

  virtual void run(Experiment *experiment) = 0;

  virtual void run(DependentVariable &group_1, DependentVariable &group_2) = 0;
  
  // A tiny wrapper for accessing test alpha
  float alpha_;
  virtual float alpha() {
    return alpha_;
  }
};

///
/// @brief      Declration of t-test.
///
/// The `run()` method will check the significance of the difference between two
/// groups. In the current setup, every `experiment->means` is considered an
/// effect size between a treatment group and a control group with the mean of
/// zero. Therefore, computing the t-statistics and computing the p-value would
/// be sufficient. This is technically an implementation of [one sample
/// t-test](https://en.wikipedia.org/wiki/Student%27s_t-test#One-sample_t-test).
///
/// @ingroup  TestStrategies
///
class TTest final : public TestStrategy {

public:
  struct Parameters {
    TestMethod name = TestMethod::TTest;
    TestAlternative alternative = TestAlternative::TwoSided;
    float alpha;
    bool var_equal {true};
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TTest::Parameters, name, alternative, alpha, var_equal);
  };

  struct ResultType {
    float tstat;
    float df;
    float pvalue;
    int side;
    bool sig;

    operator std::map<std::string, std::string>() {
      return {{"tstat", std::to_string(tstat)},
              {"df", std::to_string(df)},
              {"pvalue", std::to_string(pvalue)},
              {"side", std::to_string(side)},
              {"sig", std::to_string(sig)}};
    }

    friend std::ostream &operator<<(std::ostream &os, const ResultType &type) {
      os << "tstat: " << type.tstat << " df: " << type.df
         << " pvalue: " << type.pvalue << " side: " << type.side
         << " sig: " << type.sig;
      return os;
    }
  };

  Parameters params;

  TTest(const Parameters &p) : params{p} {
    // Setting super class' alpha!
    alpha_ = p.alpha;
  };

  virtual void run(Experiment *experiment) override;

  virtual void run(DependentVariable &group_1, DependentVariable &group_2) override{};

  static ResultType t_test(const arma::Row<float> &d1,
                           const arma::Row<float> &d2, float alpha,
                           TestStrategy::TestAlternative alternative);

  static ResultType t_test(float Sm1, float Sd1, float Sn1, float Sm2,
                           float Sd2, float Sn2, float alpha,
                           TestStrategy::TestAlternative alternative,
                           bool equal_var);
  
  static std::pair<float, bool> compute_pvalue(float tstat, float df, float alpha,
                                                TestStrategy::TestAlternative alternative);

  static ResultType
  single_sample_t_test(float M, float Sm, float Sd, unsigned Sn,
                       float alpha, TestStrategy::TestAlternative alternative);

  static ResultType
  two_samples_t_test_equal_sd(float Sm1, float Sd1, unsigned Sn1, float Sm2,
                              float Sd2, unsigned Sn2, float alpha,
                              TestStrategy::TestAlternative alternative);

  static ResultType two_samples_t_test_unequal_sd(
      float Sm1, float Sd1, unsigned Sn1, float Sm2, float Sd2,
      unsigned Sn2, float alpha, TestStrategy::TestAlternative alternative);
};

///
/// @ingroup  TestStrategies
///
class FTest final : public TestStrategy {

public:
  struct Parameters {
    TestMethod name = TestMethod::FTest;
    float alpha;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(FTest::Parameters, name, alpha);
  };

  struct ResultType {
    float fstat;
    unsigned df1;
    unsigned df2;
    float pvalue;
    bool sig;

    operator std::map<std::string, std::string>() {
      return {{"fstat", std::to_string(fstat)},
              {"df1", std::to_string(df1)},
              {"df2", std::to_string(df2)},
              {"pvalue", std::to_string(pvalue)},
              {"sig", std::to_string(sig)}};
    }

    friend std::ostream &operator<<(std::ostream &os, const ResultType &type) {
      os << "fstat: " << type.fstat << " df1: " << type.df1
         << " df2: " << type.df2 << " pvalue: " << type.pvalue
         << " sig: " << type.sig;
      return os;
    }
  };

  Parameters params;

  FTest(const Parameters &p) : params{p} {
    // Setting super class' alpha!
    alpha_ = p.alpha;
  };

  virtual void run(Experiment *experiment) override;

  virtual void run(DependentVariable &group_1, DependentVariable &group_2) override{};

  static ResultType f_test(float Sd1, unsigned Sn1, float Sd2, unsigned Sn2,
                    float alpha);
};

///
/// @ingroup  TestStrategies
///
class YuenTest final : public TestStrategy {

public:
  struct Parameters {
    TestMethod name = TestMethod::YuenTest;
    TestAlternative alternative = TestAlternative::TwoSided;
    float alpha {0.05};
    float trim {0.20};
    bool paired {false};
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(YuenTest::Parameters, name, alternative, alpha, trim, paired);
  };

  struct ResultType {
    float tstat;
    float df;
    float pvalue;
    int side;
    bool sig;

    operator std::map<std::string, std::string>() {
      return {{"tstat", std::to_string(tstat)},
              {"df", std::to_string(df)},
              {"pvalue", std::to_string(pvalue)},
              {"side", std::to_string(side)},
              {"sig", std::to_string(sig)}};
    }

    friend std::ostream &operator<<(std::ostream &os, const ResultType &type) {
      os << "tstat: " << type.tstat << " df: " << type.df
         << " pvalue: " << type.pvalue << " side: " << type.side
         << " sig: " << type.sig;
      return os;
    }
  };

  Parameters params;

  YuenTest(const Parameters &p) : params{p} {
    // Setting super class' alpha!
    alpha_ = p.alpha;
  };

  virtual void run(Experiment *experiment) override;

  virtual void run(DependentVariable &group_1, DependentVariable &group_2) override{};

  static ResultType
  yuen_t_test_one_sample(const arma::Row<float> &x, float alpha,
                         const TestStrategy::TestAlternative alternative,
                         float trim, float mu);

  static ResultType yuen_t_test_paired(
      const arma::Row<float> &x, const arma::Row<float> &y, float alpha,
      const TestStrategy::TestAlternative alternative, float trim, float mu);

  static ResultType yuen_t_test_two_samples(
      const arma::Row<float> &x, const arma::Row<float> &y, float alpha,
      const TestStrategy::TestAlternative alternative, float trim, float mu);
};

///
/// @ingroup  TestStrategies
///
class WilcoxonTest final : public TestStrategy {

public:
  struct Parameters {
    TestMethod name = TestMethod::WilcoxonTest;
    TestAlternative alternative = TestAlternative::TwoSided;
    float alpha {0.95};
    bool use_continuity{true};
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(WilcoxonTest::Parameters, name, alternative, alpha, use_continuity);
  };

  struct ResultType {
    float zstat;
    float wstat;
    float pvalue;
    int side;
    bool sig;

    operator std::map<std::string, std::string>() {
      return {{"zstat", std::to_string(zstat)},
              {"Wstat", std::to_string(wstat)},
              {"pvalue", std::to_string(pvalue)},
              {"side", std::to_string(side)},
              {"sig", std::to_string(sig)}};
    }

    friend std::ostream &operator<<(std::ostream &os, const ResultType &type) {
      os << "zstat: " << type.zstat << " wstat: " << type.wstat
         << " pvalue: " << type.pvalue << " side: " << type.side
         << " sig: " << type.sig;
      return os;
    }
  };

  Parameters params;

  WilcoxonTest(const Parameters &p) : params{p} {
    // Setting super class' alpha!
    alpha_ = p.alpha;
  };

  void run(Experiment *experiment) override;

  void run(DependentVariable &group_1, DependentVariable &group_2) override{};

  static ResultType
  wilcoxon_test(const arma::Row<float> &x, const arma::Row<float> &y,
                float alpha, float use_continuity,
                const TestStrategy::TestAlternative alternative);
};

NLOHMANN_JSON_SERIALIZE_ENUM(TestStrategy::TestMethod,
                             {{TestStrategy::TestMethod::TTest, "TTest"},
                              {TestStrategy::TestMethod::FTest, "FTest"},
                              {TestStrategy::TestMethod::YuenTest, "YuenTest"},
                              {TestStrategy::TestMethod::WilcoxonTest,
                               "WilcoxonTest"}})

NLOHMANN_JSON_SERIALIZE_ENUM(
    TestStrategy::TestAlternative,
    {{TestStrategy::TestAlternative::Less, "Less"},
     {TestStrategy::TestAlternative::Greater, "Greater"},
     {TestStrategy::TestAlternative::TwoSided, "TwoSided"}})

/// Stats Utility
float single_sample_find_df(float M, float Sm, float Sd, float alpha,
                             TestStrategy::TestAlternative alternative);

std::pair<float, float>
confidence_limits_on_mean(float Sm, float Sd, unsigned Sn, float alpha,
                          TestStrategy::TestAlternative alternative);

/// Stats Utility
float win_var(const arma::Row<float> &x, float trim);

std::pair<float, float> win_cor_cov(const arma::Row<float> &x,
                                      const arma::Row<float> &y,
                                      float trim);

arma::Row<float> win_val(const arma::Row<float> &x, float trim);

float trim_mean(const arma::Row<float> &x, float trim);

float tie_correct(const arma::Col<float> &rankval);

arma::Col<float> rankdata(const arma::Row<float> &arr, const std::string method);

template <typename T> arma::uvec nonzeros_index(const T &x) {

  return arma::find(x != 0);
}

} // namespace sam

#endif // SAMPP_TESTSTRATEGIES_H
