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
  };
  
  // \todo: To be implemented
  struct Parameters {
    std::string tau2_estimator{"DL"};
  };
  
  RandomEffectEstimator() = default;
  
  void estimate(Journal *journal);
  
  static ResultType RandomEffect(const arma::Row<double> &vi, const arma::Row<double> &yi, double tau2);
  double DL(const arma::Row<double> &yi, const arma::Row<double> &vi, const arma::Row<double> &ai);
  double PM(const arma::Row<double> &yi, const arma::Row<double> &vi, const double tau2);
};

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
    double stat;
    double pval;
    bool sig;
    double df;
  };
  
  /// \todo: to be extended! and be properly implemented
  struct Parameters {
    double alpha {0.10};
  };
  
  Parameters params;
  
  EggersTestEstimator() = default;
  
  EggersTestEstimator(const Parameters &p) : params(p) {};
  
  void estimate(Journal *journal);
  
  static ResultType EggersTest(const arma::Row<double> &yi, const arma::Row<double> &vi, double alpha);
  
};

} // namespace sam

#endif // SAMPP_METAANALYSIS_H
