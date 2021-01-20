//
// Created by Amir Masoud Abdol on 15/01/2021.
//

#ifndef SAMPP_POLICIES_VARIABLES_AND_FUNCTIONS_H
#define SAMPP_POLICIES_VARIABLES_AND_FUNCTIONS_H

#include "sol/sol.hpp"
#include "DependentVariable.h"
#include "Submission.h"
#include <random>

using namespace sam;

struct PoliciesVariablesAndFunctions {
  
  

  sol::state lua;
  
  std::vector<DependentVariable> dvs;
  std::vector<Submission> subs;
  
  PoliciesVariablesAndFunctions() {

    lua.open_libraries();

    lua.new_usertype<DependentVariable>("DependentVariable",
     "id", &DependentVariable::id_,
     "nobs", &DependentVariable::nobs_,
     "mean", &DependentVariable::mean_,
     "pvalue", &DependentVariable::pvalue_,
     "effect", &DependentVariable::effect_,
     "sig", &DependentVariable::sig_,
     "hacked", &DependentVariable::is_hacked_,
     "candidate", &DependentVariable::is_candidate_
     );

    lua.new_usertype<Submission>("Submission",
        "id", sol::property([](Submission &s) { return s.dv_.id_; }),
        "nobs", sol::property([](Submission &s) { return s.dv_.nobs_; }),
        "mean", sol::property([](Submission &s) { return s.dv_.mean_; }),
        "pvalue", sol::property([](Submission &s) { return s.dv_.pvalue_; }),
        "effect", sol::property([](Submission &s) { return s.dv_.effect_; }),
        "sig", sol::property([](Submission &s) { return s.dv_.sig_; }),
        "hacked", sol::property([](Submission &s) { return s.dv_.is_hacked_; }),
        "candidate", sol::property([](Submission &s) { return s.dv_.is_candidate_; }));
    
  }
  
  std::vector<std::string> quantitative_variables {
  	"id",
  	"nobs",
  	"mean",
    "pvalue",
    "effect"
  };

  std::vector<std::string> meta_variables {
    "sig",
    "hacked",
    "candidate"
  };

  std::vector<std::string> binary_operators {
  	">",
  	">=",
  	"<",
  	"<=",
  	"==",
  	"!="
  };

  std::vector<std::string> unary_functions {
  	"min",
  	"max",
  	"random",
  	"first",
  	"last",
    "all"
  };

};

#endif // SAMPP_POLICIES_VARIABLES_AND_FUNCTIONS_H
