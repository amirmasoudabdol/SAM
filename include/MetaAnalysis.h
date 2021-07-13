//
// Created by Amir Masoud Abdol on 2019-04-25.
//
//

/// @defgroup   MetaAnalysis Meta Analysis Methods
/// @brief      List of available Meta Analysis
///
/// Description to come!
///

/// @defgroup   MetaAnalysisParameters Parameters of Meta Anlaysis Methods
/// @brief      List of available Parameters
///
/// Description to come!
///

#ifndef SAMPP_METAANALYSIS_H
#define SAMPP_METAANALYSIS_H

#include <ostream>
#include <vector>

#include "sam.h"

#include "Experiment.h"
#include "Submission.h"


namespace sam {

using namespace std;

class Journal;

class MetaAnalysis {

public:

  virtual ~MetaAnalysis() = 0;

  static std::unique_ptr<MetaAnalysis> build(std::string name);
  
  static std::unique_ptr<MetaAnalysis> build(const json &config);
  
  static std::vector<std::string> Columns(std::string name);
  
  virtual void estimate(Journal *journal) = 0;
};

/// @brief  Random Effect Estimator
///
/// @ingroup  MetaAnalysis
///
class RandomEffectEstimator : public MetaAnalysis {

public:
  
  struct ResultType {
    float est;
    float se;
    float ci_lb;
    float ci_ub;
    float zval;
    float pval;
    float q_stat;
    float q_pval;
    
    float tau2;
    
    ResultType() = default;

    friend ostream &operator<<(ostream &os, const ResultType &type) {
      os << "est: " << type.est << " se: " << type.se
         << " ci_lb: " << type.ci_lb << " ci_ub: " << type.ci_ub
         << " zval: " << type.zval << " pval: " << type.pval
         << " q_stat: " << type.q_stat << " q_pval: " << type.q_pval
         << " tau2: " << type.tau2;
      return os;
    }
    
    static std::vector<std::string> Columns() {
      return {"est", "se", "ci_lb", "ci_ub", "zval", "pval", "q_stat", "q_pval", "tau2"};
    }
    
    operator std::vector<std::string>() {
      return {
        std::to_string(est),
        std::to_string(se),
        std::to_string(ci_lb),
        std::to_string(ci_ub),
        std::to_string(zval),
        std::to_string(pval),
        std::to_string(q_stat),
        std::to_string(q_pval),
        std::to_string(tau2)
      };
    }
    
    operator std::map<std::string, std::string>() {
      return {
        {"est", std::to_string(est)},
        {"se", std::to_string(se)},
        {"ci_lb", std::to_string(ci_lb)},
        {"ci_ub", std::to_string(ci_ub)},
        {"zval", std::to_string(zval)},
        {"pval", std::to_string(pval)},
        {"q_stat", std::to_string(q_stat)},
        {"q_pval", std::to_string(q_pval)},
        {"tau2", std::to_string(tau2)}
      };
    }
    
    operator arma::Row<float>() {
      return {
          est,
          se,
          ci_lb,
          ci_ub,
          zval,
          pval,
          q_stat,
          q_pval,
          tau2
      };
    }
    
  };
  
  /// @brief Parameters of the RandomEffectEstimator
  ///
  /// @ingroup MetaAnalysisParameters
  struct Parameters {
    std::string name{"RandomEffectEstimator"};
    
    //! The random effect estimator
    std::string estimator{"DL"};
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RandomEffectEstimator::Parameters, name, estimator);
  };
  
  Parameters params;
  
  RandomEffectEstimator() = default;
  
  RandomEffectEstimator(const Parameters &p) : params{p} {};
  
  void estimate(Journal *journal);
  
  static ResultType RandomEffect(const arma::Row<float> &vi, const arma::Row<float> &yi, float tau2);
  float DL(const arma::Row<float> &yi, const arma::Row<float> &vi, const arma::Row<float> &ai);
  float PM(const arma::Row<float> &yi, const arma::Row<float> &vi, const float tau2);
};


/// @brief Fixed Effect Estimator
///
/// @ingroup  MetaAnalysis
///
class FixedEffectEstimator : public MetaAnalysis {
  
public:
  
  struct ResultType {
    float est;
    float se;
    float ci_lb;
    float ci_ub;
    float zval;
    float pval;
    float q_stat;
    float q_pval;
    
    float tau2;
    
    ResultType() = default;
    
    ResultType(const RandomEffectEstimator::ResultType &res) {
      est = res.est;
      se = res.se;
      ci_lb = res.ci_lb;
      ci_ub = res.ci_ub;
      zval = res.zval;
      pval = res.pval;
      q_stat = res.q_stat;
      q_pval = res.q_pval;
      
      tau2 = 0;
    }
    
    static std::vector<std::string> Columns() {
      return {"est", "se", "ci_lb", "ci_ub", "zval", "pval", "q_stat", "q_pval", "tau2"};
    }
    
    operator std::vector<std::string>() {
      return {
        std::to_string(est),
        std::to_string(se),
        std::to_string(ci_lb),
        std::to_string(ci_ub),
        std::to_string(zval),
        std::to_string(pval),
        std::to_string(q_stat),
        std::to_string(q_pval),
        std::to_string(tau2)
      };
    }
    
    operator arma::Row<float>() {
      return {
        est,
        se,
        ci_lb,
        ci_ub,
        zval,
        pval,
        q_stat,
        q_pval,
        tau2
      };
    }

    friend ostream &operator<<(ostream &os, const ResultType &type) {
      os << "est: " << type.est << " se: " << type.se
         << " ci_lb: " << type.ci_lb << " ci_ub: " << type.ci_ub
         << " zval: " << type.zval << " pval: " << type.pval
         << " q_stat: " << type.q_stat << " q_pval: " << type.q_pval
         << " tau2: " << type.tau2;
      return os;
    }
  };
  
  FixedEffectEstimator() = default;
  
  void estimate(Journal *journal);
  
  static ResultType FixedEffect(const arma::Row<float> &yi, const arma::Row<float> &vi) {
    return RandomEffectEstimator::RandomEffect(yi, vi, 0);
  }
};

///
/// @brief Egger's Test of Funnel Plot Symmetry
///
/// A strategy performing Egger's Test on the available list of Journal's publications.
/// List of available parameters and the format of output can be found in Parameters, and ResultType.
///
/// @ingroup  MetaAnalysis
///
class EggersTestEstimator : public MetaAnalysis {
public:
  
  /// Egger's Test Output
  ///
  struct ResultType {
    //! The slope of the fitted line
    float slope;
    
    //! The standard error of the slope
    float se;
    
    //! The t-statistic of the test
    float tstat;
    
    //! The p-value of the test
    float pval;
    
    //! The significance of the test
    bool sig;
    
    //! The degree-of-freedom of the test
    float df;
    
    static std::vector<std::string> Columns() {
      return {"slope", "se", "tstat", "pval", "sig", "df"};
    }
    
    operator std::vector<std::string>() {
      return {
        std::to_string(slope),
        std::to_string(se),
        std::to_string(tstat),
        std::to_string(pval),
        std::to_string(sig),
        std::to_string(df)
      };
    }
    
    operator arma::Row<float>() {
      return {
        slope,
        se,
        tstat,
        pval,
        static_cast<float>(sig),
        df
      };
    }
    
  };
  
  /// @brief Parameters of the EggersTestEstimator
  ///
  /// While Egger's test is often performed with ɑ = 0.1, it's possible to
  /// change the ɑ using the #alpha parameter.
  ///
  /// @ingroup MetaAnalysisParameters
  struct Parameters {
    std::string name {"EggersTestEstimator"};
    
    //! The ɑ of the test
    float alpha {0.10};
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(EggersTestEstimator::Parameters, name, alpha);
  };
  
  Parameters params;
  
  EggersTestEstimator() = default;
  
  EggersTestEstimator(const Parameters &p) : params(p) {};
  
  void estimate(Journal *journal);
  
  static ResultType EggersTest(const arma::Row<float> &yi, const arma::Row<float> &vi, float alpha);
  
};


/// @brief Test of Excess of Significant Findings
///
/// @ingroup  MetaAnalysis
///
class TestOfObsOverExptSig : public MetaAnalysis {
public:
  
  struct ResultType {
    //! Sum of the expected probabilities
    float E;
    
    //! The chi-square statistic of the test
    float A;
    
    //! The p-value of the test
    float pval;
    
    //! The significance of the test
    bool sig;
    
    static std::vector<std::string> Columns() {
      return {"E", "A", "pval", "sig"};
    }
    
    operator std::vector<std::string>() {
      return {
        std::to_string(E),
        std::to_string(A),
        std::to_string(pval),
        std::to_string(sig),
      };
    }
    
    operator arma::Row<float>() {
      return {
        E,
        A,
        pval,
        static_cast<float>(sig)
      };
    }
    
  };
  
  /// Parameters of TestOfObsOverExptSig
  ///
  /// @ingroup MetaAnalysisParameters
  struct Parameters {
    std::string name {"TestOfObsOverExptSig"};
    float alpha {0.10};
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TestOfObsOverExptSig::Parameters, name, alpha);
  };
  
  Parameters params;
  
  TestOfObsOverExptSig() = default;
  
  TestOfObsOverExptSig(const Parameters &p) : params(p) {};
  
  void estimate(Journal *journal);
  
  static ResultType TES(const arma::Row<float> &sigs, const arma::Row<float> &ni, float beta, float alpha);
  
};

/// @brief Trim and Fill Publication Bias Test
///
/// @ingroup  MetaAnalysis
///
class TrimAndFill : public MetaAnalysis {
public:
  
  struct ResultType {
    float k0;
    float se_k0;
    float k_all;
    std::string side;
    float imputed_est;
    float imputed_pval;
//    std::optional<float> k0_pval;  // I don't report this yet
    
    static std::vector<std::string> Columns() {
      return {"k0", "se_k0", "k_all", "side", "imputed_est", "imputed_pval"};
    }
    
    operator std::vector<std::string>() {
      return {
        std::to_string(k0),
        std::to_string(se_k0),
        std::to_string(k_all),
        side,
        std::to_string(imputed_est),
        std::to_string(imputed_pval)
      };
    }
    
    operator arma::Row<float>() {
      return {
        k0,
        se_k0,
        k_all,
        static_cast<float>(1. ? (side.find("left") != std::string::npos) : -1.),
        imputed_est,
        imputed_pval
      };
    }
  };
  
  /// @brief Parameters of the TrimAndFill
  struct Parameters {
    std::string name {"TrimAndFill"};
    
    //! Indicates the side of the funnel plot where missing values should be imputed
    std::string side {"auto"};
    
    //! The symmetry estimator
    std::string estimator {"R0"};
    
    //! The ɑ of the test
    float alpha {0.10};
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TrimAndFill::Parameters, name, side, estimator, alpha);
  };
  
  Parameters params;
  
  TrimAndFill() = default;
  
  TrimAndFill(const Parameters &p) : params(p) {};
  
  void estimate(Journal *journal);
  
  static ResultType TF(arma::Row<float> yi, arma::Row<float> vi, arma::Row<float> ni, const Parameters &params);
  
};

/// @brief Begg's Rank Correlation Test
///
/// @ingroup  MetaAnalysis
///
class RankCorrelation : public MetaAnalysis {
public:
  
  struct ResultType {
    //! Kendall's tau estimate
    float est;
    
    //! The p-value of the test
    float pval;
    
    //! The significance of the test
    bool sig;
    
    static std::vector<std::string> Columns() {
      return {"est", "pval", "sig"};
    }
    
    operator std::vector<std::string>() {
      return {
        std::to_string(est),
        std::to_string(pval),
        std::to_string(sig)
      };
    }
    
    operator arma::Row<float>() {
      return {
        est,
        pval,
        static_cast<float>(sig)
      };
    }
    
  };
  
  /// @brief Parameters of RankCorrelation
  ///
  /// @ingroup MetaAnalysisParameters
  struct Parameters {
    std::string name{"RankCorrelation"};
    
    TestStrategy::TestAlternative alternative = TestStrategy::TestAlternative::TwoSided;
    
    //! The ɑ of the test
    float alpha {0.10};
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RankCorrelation::Parameters, name, alternative, alpha);
  };
  
  Parameters params;
  
  RankCorrelation() = default;
  
  RankCorrelation(const Parameters &p) : params(p) {};
  
  void estimate(Journal *journal);
  
  static ResultType RankCor(arma::Row<float> yi, arma::Row<float> vi, const Parameters &params);
  
  static arma::Row<int> duplicate_count(arma::Row<float> x);
  
};


float kendallcor(const arma::Row<float> &x, const arma::Row<float> &y);

std::pair<float, float> kendall_cor_test(const arma::Row<float> &x, const arma::Row<float> &y, const TestStrategy::TestAlternative alternative);

float ckendall(int k, int n, float **w);

void pkendall(int len, float *Q, float *P, int n);

} // namespace sam

#endif // SAMPP_METAANALYSIS_H
