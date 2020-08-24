//
// Created by Amir Masoud Abdol on 2019-04-25.
//

#ifndef SAMPP_METAANALYSIS_H
#define SAMPP_METAANALYSIS_H

#include <ostream>
#include <vector>

#include "sam.h"

#include "Experiment.h"
#include "Submission.h"
//#include "Journal.h"

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

class RandomEffectEstimator : public MetaAnalysis {

public:
  
  struct ResultType {
    double est;
    double se;
    double ci_lb;
    double ci_ub;
    double zval;
    double pval;
    double q_stat;
    double q_pval;
    
    double tau2;
    
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
    
    operator arma::Row<double>() {
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
  
  // \todo: To be implemented
  struct Parameters {
    std::string name{"RandomEffectEstimator"};
    std::string estimator{"DL"};
  };
  
  Parameters params;
  
  RandomEffectEstimator() = default;
  
  RandomEffectEstimator(const Parameters &p) : params{p} {};
  
  void estimate(Journal *journal);
  
  static ResultType RandomEffect(const arma::Row<double> &vi, const arma::Row<double> &yi, double tau2);
  double DL(const arma::Row<double> &yi, const arma::Row<double> &vi, const arma::Row<double> &ai);
  double PM(const arma::Row<double> &yi, const arma::Row<double> &vi, const double tau2);
};


inline void to_json(json &j, const RandomEffectEstimator::Parameters &p) {
  j = json{
    {"name", p.name}, {"estimator", p.estimator}};
}

inline void from_json(const json &j, RandomEffectEstimator::Parameters &p) {
  
  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);
  j.at("estimator").get_to(p.estimator);
}

class FixedEffectEstimator : public MetaAnalysis {
  
public:
  
  struct ResultType {
    double est;
    double se;
    double ci_lb;
    double ci_ub;
    double zval;
    double pval;
    double q_stat;
    double q_pval;
    
    double tau2;
    
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
    
    operator arma::Row<double>() {
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
  
  static ResultType FixedEffect(const arma::Row<double> &yi, const arma::Row<double> &vi) {
    return RandomEffectEstimator::RandomEffect(yi, vi, 0);
  }
};

class EggersTestEstimator : public MetaAnalysis {
public:
  
  struct ResultType {
    double slope;
    double se;
    double tstat;
    double pval;
    bool sig;
    double df;
    
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
    
    operator arma::Row<double>() {
      return {
        slope,
        se,
        tstat,
        pval,
        static_cast<double>(sig),
        df
      };
    }
    
  };
  
  /// \todo: to be extended! and be properly implemented
  struct Parameters {
    std::string name {"EggersTestEstimator"};
    double alpha {0.10};
  };
  
  Parameters params;
  
  EggersTestEstimator() = default;
  
  EggersTestEstimator(const Parameters &p) : params(p) {};
  
  void estimate(Journal *journal);
  
  static ResultType EggersTest(const arma::Row<double> &yi, const arma::Row<double> &vi, double alpha);
  
};

inline void to_json(json &j, const EggersTestEstimator::Parameters &p) {
  j = json{
    {"name", p.name}, {"alpha", p.alpha}};
}

inline void from_json(const json &j, EggersTestEstimator::Parameters &p) {
  
  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);
  j.at("alpha").get_to(p.alpha);
}

class TestOfObsOverExptSig : public MetaAnalysis {
public:
  
  struct ResultType {
    double E;
    double A;
    double pval;
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
    
    operator arma::Row<double>() {
      return {
        E,
        A,
        pval,
        static_cast<double>(sig)
      };
    }
    
  };
  
  /// \todo: to be extended! and be properly implemented
  struct Parameters {
    std::string name {"TestOfObsOverExptSig"};
    double alpha {0.10};
  };
  
  Parameters params;
  
  TestOfObsOverExptSig() = default;
  
  TestOfObsOverExptSig(const Parameters &p) : params(p) {};
  
  void estimate(Journal *journal);
  
  static ResultType TES(const arma::Row<double> &sigs, const arma::Row<double> &ni, double beta, double alpha);
  
};


inline void to_json(json &j, const TestOfObsOverExptSig::Parameters &p) {
  j = json{
    {"name", p.name}, {"alpha", p.alpha}};
}

inline void from_json(const json &j, TestOfObsOverExptSig::Parameters &p) {
  
  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);
  j.at("alpha").get_to(p.alpha);
}

class TrimAndFill : public MetaAnalysis {
public:
  
  struct ResultType {
    double k0;
    double se_k0;
    double k_all;
    std::string side;
    double imputed_est;
    double imputed_pval;
//    std::optional<double> k0_pval;  // I don't report this yet
    
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
    
    operator arma::Row<double>() {
      return {
        k0,
        se_k0,
        k_all,
        1. ? (side.find("left") != std::string::npos) : -1.,
        imputed_est,
        imputed_pval
      };
    }
  };
  
  /// \todo: to be extended! and be properly implemented
  struct Parameters {
    std::string name {"TrimAndFill"};
    std::string side {"auto"};
    std::string estimator {"R0"};
    double alpha {0.10};
  };
  
  Parameters params;
  
  TrimAndFill() = default;
  
  TrimAndFill(const Parameters &p) : params(p) {};
  
  void estimate(Journal *journal);
  
  static ResultType TF(arma::Row<double> yi, arma::Row<double> vi, arma::Row<double> ni, const Parameters &params);
  
};

inline void to_json(json &j, const TrimAndFill::Parameters &p) {
  j = json{
    {"name", p.name}, {"side", p.side}, {"estimator", p.estimator}, {"alpha", p.alpha}};
}

inline void from_json(const json &j, TrimAndFill::Parameters &p) {
  
  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);
  j.at("side").get_to(p.side);
  j.at("estimator").get_to(p.estimator);
  j.at("alpha").get_to(p.alpha);
}


class RankCorrelation : public MetaAnalysis {
public:
  
  struct ResultType {
    double est;
    double pval;
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
    
    operator arma::Row<double>() {
      return {
        est,
        pval,
        static_cast<double>(sig)
      };
    }
    
  };
  
  /// \todo: to be extended! and be properly implemented
  struct Parameters {
    std::string name{"RankCorrelation"};
    TestStrategy::TestAlternative alternative = TestStrategy::TestAlternative::TwoSided;
    double alpha {0.10};
  };
  
  Parameters params;
  
  RankCorrelation() = default;
  
  RankCorrelation(const Parameters &p) : params(p) {};
  
  void estimate(Journal *journal);
  
  static ResultType RankCor(arma::Row<double> yi, arma::Row<double> vi, const Parameters &params);
  
  static arma::Row<int> duplicate_count(arma::rowvec x);
  
};

inline void to_json(json &j, const RankCorrelation::Parameters &p) {
  j = json{
    {"name", p.name}, {"alternative", p.alternative}, {"alpha", p.alpha}};
}

inline void from_json(const json &j, RankCorrelation::Parameters &p) {
  
  // Using a helper template function to handle the optional and throw if
  // necessary.
  j.at("name").get_to(p.name);
  j.at("alternative").get_to(p.alternative);
  j.at("alpha").get_to(p.alpha);
}



double kendallcor(const arma::Row<double> &x, const arma::Row<double> &y);

std::pair<double, double> kendall_cor_test(const arma::Row<double> &x, const arma::Row<double> &y, const TestStrategy::TestAlternative alternative);

double ckendall(int k, int n, double **w);

void pkendall(int len, double *Q, double *P, int n);

} // namespace sam

#endif // SAMPP_METAANALYSIS_H
