//
// Created by Amir Masoud Abdol on 15/01/2021.
//

#ifndef SAMPP_POLICIES_VARIABLES_AND_FUNCTIONS_H
#define SAMPP_POLICIES_VARIABLES_AND_FUNCTIONS_H

#include "sol/sol.hpp"
#include "DependentVariable.h"
#include "Submission.h"

using namespace sam;

struct PoliciesVariablesAndFunctions {

  sol::state lua;
  
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
     "candidated", &DependentVariable::is_candidate_
     );

    lua.new_usertype<Submission>("Submission",
        "id", sol::property([](Submission &s) { return s.group_.id_; }),
        "nobs", sol::property([](Submission &s) { return s.group_.nobs_; }),
        "mean", sol::property([](Submission &s) { return s.group_.mean_; }),
        "pvalue", sol::property([](Submission &s) { return s.group_.pvalue_; }),
        "effect", sol::property([](Submission &s) { return s.group_.effect_; }),
        "sig", sol::property([](Submission &s) { return s.group_.sig_; }),
        "hacked", sol::property([](Submission &s) { return s.group_.is_hacked_; }),
        "candidated", sol::property([](Submission &s) { return s.group_.is_candidate_; }));
    
  }

  
  std::vector<std::string> binary_variables {
  	"id",
  	"nobs",
  	"mean",
	"pvalue",
	"effect"
  };

  std::vector<std::string> unary_variables {
	"sig",
	"hacked",
	"candidated"
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
  	"last"
  };

};

#endif // SAMPP_POLICIES_VARIABLES_AND_FUNCTIONS_H
