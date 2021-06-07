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
  
  std::vector<float> values {
    0, -0, .0, -.0, 0.0, -0.0,
    .1, -.1, 0.1, -0.1,
    1, -1
  };
  
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
  

  sol::state lua;
  
  std::vector<std::string> all_possible_binary_policies;
  std::vector<std::string> all_possible_unary_policies;
  std::vector<std::string> all_possible_meta_policies;
  std::vector<std::string> all_possible_policies;
  
  std::vector<std::string> some_failed_policies;
  
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
    
    for (auto &var : meta_variables) {
      all_possible_meta_policies.push_back(var);
      all_possible_meta_policies.push_back("!" + var);
    }
    
    for (auto &var : quantitative_variables) {
      for (auto &op : binary_operators) {
        for (auto &val : values) {
          all_possible_binary_policies.push_back(var + " " + op + " " + std::to_string(val));
        }
      }
      
      for (auto &op : unary_functions) {
        all_possible_unary_policies.push_back(op + "(" + var + ")");
      }
    }
    
    all_possible_policies.insert(all_possible_policies.begin(),
                                 all_possible_meta_policies.begin(),
                                 all_possible_meta_policies.end());
    
    all_possible_policies.insert(all_possible_policies.begin(),
                                 all_possible_binary_policies.begin(),
                                 all_possible_binary_policies.end());
    
    all_possible_policies.insert(all_possible_policies.begin(),
                                 all_possible_unary_policies.begin(),
                                 all_possible_unary_policies.end());
    
  }
  


};

#endif // SAMPP_POLICIES_VARIABLES_AND_FUNCTIONS_H
