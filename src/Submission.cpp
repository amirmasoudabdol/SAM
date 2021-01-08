//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include "Submission.h"

namespace sam {

Submission::Submission(Experiment &e, const int &index) {
  
  tnobs = e[index].true_nobs_.value();

  group_ = e[index];
 
  simid = e.simid;
  exprid = e.exprid;
  repid = e.repid;
}

std::vector<std::string> Submission::Columns() {

  std::vector<std::string> cols {"simid", "exprid", "repid", "pubid", "tnobs"};
  auto group_cols = Group::Columns();
  cols.insert(cols.end(), group_cols.begin(), group_cols.end());
  
  return cols;
}

Submission::operator std::map<std::string, std::string>() {
  
  record["simid"] = std::to_string(simid);
  record["exprid"] = std::to_string(exprid);
  record["repid"] = std::to_string(repid);
  record["pubid"] = std::to_string(pubid);

  record["tnobs"] = std::to_string(tnobs);

  std::map<std::string, std::string> g_record {group_};
  record.insert(g_record.begin(), g_record.end());

  return record;
}

Submission::operator arma::Row<double>() {
  
  arma::Row<double> row {
    static_cast<double>(simid),
    static_cast<double>(exprid),
    static_cast<double>(repid),
    static_cast<double>(pubid),
    static_cast<double>(tnobs)};
  
  row.insert_cols(row.n_elem, static_cast<arma::Row<double>>(group_));
  
  return row;
}

} // namespace sam
