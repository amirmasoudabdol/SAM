//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_TESTSTRATEGIES_H
#define SAMPP_TESTSTRATEGIES_H

#include "Experiment.h"
#include "Utilities.h"
#include <ostream>

namespace sam {

class Group;
class ExperimentSetup;
class Experiment;

///
/// \brief      Abstract class for defining test strategies.
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

  ///
  /// Contains Test Strategy parameters.
  ///
  /// \note       I'm still experimenting with this while I'm trying to simplify
  ///             the construction process.
  ///
  struct TestStrategyParameters {
    TestMethod name;
    TestAlternative alternative = TestAlternative::TwoSided;
    double alpha = 0.05;
  };
  //
  //        TestStrategyParameters params;

  static std::unique_ptr<TestStrategy> build(json &test_strategy_config);

  virtual void run(Experiment *experiment) = 0;

  virtual void run(Group &group_1, Group &group_2) = 0;
};

///
/// \brief      Declration of t-test.
///
/// The `run()` method will check the significance of the difference between two
/// groups. In the current setup, every `experiment->means` is considered an
/// effect size between a treatment group and a control group with the mean of
/// zero. Therefore, computing the t-statistics and computing the p-value would
/// be sufficient. This is technically an implementation of [one sample
/// t-test](https://en.wikipedia.org/wiki/Student%27s_t-test#One-sample_t-test).
///
class TTest final : public TestStrategy {

public:
  struct Parameters {
    TestMethod name = TestMethod::TTest;
    TestAlternative alternative = TestAlternative::TwoSided;
    double alpha;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TTest::Parameters, name, alternative, alpha);
  };

  struct ResultType {
    double tstat;
    double df;
    double pvalue;
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

  TTest(const Parameters &p) : params{p} {};

  // Cleanup
  TTest(TestStrategyParameters tsp){
      //            params = tsp;
  };

  virtual void run(Experiment *experiment) override;

  virtual void run(Group &group_1, Group &group_2) override{};

  static ResultType t_test(const arma::Row<double> &d1,
                           const arma::Row<double> &d2, double alpha,
                           TestStrategy::TestAlternative alternative);

  static ResultType t_test(double Sm1, double Sd1, double Sn1, double Sm2,
                           double Sd2, double Sn2, double alpha,
                           TestStrategy::TestAlternative alternative,
                           bool equal_var);
  
  static std::pair<double, bool> compute_pvalue(double tstat, double df, double alpha,
                                                TestStrategy::TestAlternative alternative);

  static ResultType
  single_sample_t_test(double M, double Sm, double Sd, unsigned Sn,
                       double alpha, TestStrategy::TestAlternative alternative);

  static ResultType
  two_samples_t_test_equal_sd(double Sm1, double Sd1, unsigned Sn1, double Sm2,
                              double Sd2, unsigned Sn2, double alpha,
                              TestStrategy::TestAlternative alternative);

  static ResultType two_samples_t_test_unequal_sd(
      double Sm1, double Sd1, unsigned Sn1, double Sm2, double Sd2,
      unsigned Sn2, double alpha, TestStrategy::TestAlternative alternative);
};


//inline void to_json(json &j, const TTest::Parameters &p) {
//  j = json{
//      {"name", p.name}, {"alternative", p.alternative}, {"alpha", p.alpha}};
//}
//
//inline void from_json(const json &j, TTest::Parameters &p) {
//
//  // Using a helper template function to handle the optional and throw if
//  // necessary.
//  j.at("name").get_to(p.name);
//
//  j.at("alternative").get_to(p.alternative);
//
//  j.at("alpha").get_to(p.alpha);
//}

class FTest final : public TestStrategy {

public:
  struct Parameters {
    TestMethod name = TestMethod::FTest;
    double alpha;
  };

  struct ResultType {
    double fstat;
    unsigned df1;
    unsigned df2;
    double pvalue;
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

  FTest(const Parameters &p) : params{p} {};

  // Cleanup
  FTest(TestStrategyParameters tsp){
      //            params = tsp;
  };

  virtual void run(Experiment *experiment) override;

  virtual void run(Group &group_1, Group &group_2) override{};

  static ResultType f_test(double Sd1, unsigned Sn1, double Sd2, unsigned Sn2,
                    double alpha);
};


inline void to_json(json &j, const FTest::Parameters &p) {
  j = json{{"name", p.name}, {"alpha", p.alpha}};
}

inline void from_json(const json &j, FTest::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);

  j.at("alpha").get_to(p.alpha);
}

class YuenTest final : public TestStrategy {

public:
  struct Parameters {
    TestMethod name = TestMethod::YuenTest;
    TestAlternative alternative = TestAlternative::TwoSided;
    double alpha = 0.95;
    double trim = 0.20;
  };

  struct ResultType {
    double tstat;
    double df;
    double pvalue;
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

  YuenTest(const Parameters &p) : params{p} {};

  // Cleanup
  YuenTest(TestStrategyParameters tsp){
      //            params = tsp;
  };

  virtual void run(Experiment *experiment) override;

  virtual void run(Group &group_1, Group &group_2) override{};

  static ResultType
  yuen_t_test_one_sample(const arma::Row<double> &x, double alpha,
                         const TestStrategy::TestAlternative alternative,
                         double trim, double mu);

  static ResultType yuen_t_test_paired(
      const arma::Row<double> &x, const arma::Row<double> &y, double alpha,
      const TestStrategy::TestAlternative alternative, double trim, double mu);

  static ResultType yuen_t_test_two_samples(
      const arma::Row<double> &x, const arma::Row<double> &y, double alpha,
      const TestStrategy::TestAlternative alternative, double trim, double mu);
};


inline void to_json(json &j, const YuenTest::Parameters &p) {
  j = json{
      {"name", p.name}, {"alternative", p.alternative}, {"alpha", p.alpha}
      // ,
      // {"trim", p.trim}
  };
}

inline void from_json(const json &j, YuenTest::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);

  j.at("alternative").get_to(p.alternative);

  j.at("alpha").get_to(p.alpha);
  // j.at("trim").get_to(p.trim);
}

class WilcoxonTest final : public TestStrategy {

public:
  struct Parameters {
    TestMethod name = TestMethod::WilcoxonTest;
    TestAlternative alternative = TestAlternative::TwoSided;
    double alpha = 0.95;
    bool use_continuity{true};
  };

  struct ResultType {
    double zstat;
    double wstat;
    double pvalue;
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

  WilcoxonTest(const Parameters &p) : params{p} {};

  // Cleanup
  WilcoxonTest(TestStrategyParameters tsp){
      //            params = tsp;
  };

  virtual void run(Experiment *experiment) override;

  virtual void run(Group &group_1, Group &group_2) override{};

  static ResultType
  wilcoxon_test(const arma::Row<double> &x, const arma::Row<double> &y,
                double alpha, double use_continuity,
                const TestStrategy::TestAlternative alternative);
};


inline void to_json(json &j, const WilcoxonTest::Parameters &p) {
  j = json{
      {"name", p.name}, {"alternative", p.alternative}, {"alpha", p.alpha}
      // , {"use_continuity", p.use_continuity}
  };
}

inline void from_json(const json &j, WilcoxonTest::Parameters &p) {

  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);

  j.at("alternative").get_to(p.alternative);

  j.at("alpha").get_to(p.alpha);

  // j.at("use_continuity").get_to(p.use_continuity);
}

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
double single_sample_find_df(double M, double Sm, double Sd, double alpha,
                             TestStrategy::TestAlternative alternative);

std::pair<double, double>
confidence_limits_on_mean(double Sm, double Sd, unsigned Sn, double alpha,
                          TestStrategy::TestAlternative alternative);

/// Stats Utility
double win_var(const arma::Row<double> &x, const double trim);

std::pair<double, double> win_cor_cov(const arma::Row<double> &x,
                                      const arma::Row<double> &y,
                                      const double trim);

arma::Row<double> win_val(const arma::Row<double> &x, double trim);

double trim_mean(const arma::Row<double> &x, double trim);

double tie_correct(const arma::vec &rankval);

arma::vec rankdata(const arma::Row<double> &arr, const std::string method);

template <typename T> arma::uvec nonzeros_index(const T &x) {

  return arma::find(x != 0);
}

} // namespace sam

#endif // SAMPP_TESTSTRATEGIES_H
