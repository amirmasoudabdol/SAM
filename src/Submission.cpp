//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include "Submission.h"

namespace sam {

Submission::Submission(Experiment &e, const int &index) {

  // This is an ugly hack to solve the GroupPooling problem, I need to move
  // this to the Experiment
  if (index < e.setup.ng()) {
    tnobs = e.setup.nobs()[index];
  } else {
    tnobs = 0;
  }

  group_ = e[index];
};

std::vector<std::string> Submission::cols() {

  if (!record.empty()) {
    for (auto &item : record) {
      columns.push_back(item.first);
    }
  }
  return columns;
}

Submission::operator std::map<std::string, std::string>() {

  record["simid"] = std::to_string(simid);
  record["pubid"] = std::to_string(pubid);

  record["tnobs"] = std::to_string(tnobs);

  std::map<std::string, std::string> g_record = group_;
  record.insert(g_record.begin(), g_record.end());

  return record;
}

} // namespace sam
