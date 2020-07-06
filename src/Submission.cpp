//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include "Submission.h"

namespace sam {

Submission::Submission(Experiment &e, const int &index) {
  
  // TODO: This can be simplified further
  tnobs = e[index].true_nobs_.value();

  group_ = e[index];
  
  repid = e.repid;
};

std::vector<std::string> Submission::cols() {

  if (!record.empty()) {
    for (const auto &item : record) {
      columns.push_back(item.first);
    }
  }
  return columns;
}

Submission::operator std::map<std::string, std::string>() {

  record["simid"] = std::to_string(simid);
  record["repid"] = std::to_string(repid);
  record["pubid"] = std::to_string(pubid);

  record["tnobs"] = std::to_string(tnobs);

  std::map<std::string, std::string> g_record = group_;
  record.insert(g_record.begin(), g_record.end());

  return record;
}

} // namespace sam
