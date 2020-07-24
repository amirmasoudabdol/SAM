//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include "Submission.h"

namespace sam {

Submission::Submission(Experiment &e, const int &index) {
  
  /// \todo This can be simplified further
  tnobs = e[index].true_nobs_.value();

  group_ = e[index];
  
  exprid = e.exprid;
  repid = e.repid;
};

std::vector<std::string> Submission::Columns() {

  std::vector<std::string> cols {"simid", "exprid", "repid", "pubid", "tnobs"};
  auto group_cols = GroupData::Columns();
  cols.insert(cols.end(), group_cols.begin(), group_cols.end());
  
  return cols;
}

Submission::operator std::map<std::string, std::string>() {

  record["simid"] = std::to_string(simid);
  record["exprid"] = std::to_string(exprid);
  record["repid"] = std::to_string(repid);
  record["pubid"] = std::to_string(pubid);

  record["tnobs"] = std::to_string(tnobs);

  std::map<std::string, std::string> g_record = group_;
  record.insert(g_record.begin(), g_record.end());

  return record;
}

} // namespace sam
