//===-- Submission.cpp - Implementation of Submission ---------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-01-25.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the implementation of Submission.
///
//===----------------------------------------------------------------------===//

#include "Submission.h"

namespace sam {

Submission::Submission(Experiment &e, const int &index) {
  
  dv_ = e[index];
 
  simid = e.simid;
  exprid = e.exprid;
  repid = e.repid;
}

Submission::Submission(int sim_id, int expr_id, int rep_id, int pub_id, DependentVariable dv)
  : simid{sim_id}, exprid{expr_id}, repid{rep_id}, pubid{pub_id},
      dv_{dv} {

}

std::vector<std::string> Submission::Columns() {

  std::vector<std::string> cols {"simid", "exprid", "repid", "pubid"};
  auto group_cols = DependentVariable::Columns();
  cols.insert(cols.end(), group_cols.begin(), group_cols.end());
  
  return cols;
}

Submission::operator std::map<std::string, std::string>() {
  
  std::map<std::string, std::string> record;
  
  record["simid"] = std::to_string(simid);
  record["exprid"] = std::to_string(exprid);
  record["repid"] = std::to_string(repid);
  record["pubid"] = std::to_string(pubid);

  std::map<std::string, std::string> g_record {dv_};
  record.insert(g_record.begin(), g_record.end());

  return record;
}

Submission::operator arma::Row<float>() {
  
  arma::Row<float> row {
    static_cast<float>(simid),
    static_cast<float>(exprid),
    static_cast<float>(repid),
    static_cast<float>(pubid)};
  
  row.insert_cols(row.n_elem, static_cast<arma::Row<float>>(dv_));
  
  return row;
}

} // namespace sam
