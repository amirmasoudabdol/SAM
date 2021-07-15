//
// Created by Amir Masoud Abdol on 2019-06-03.
//

#include <iostream>
#include <memory>

#include "csv/reader.hpp"
#include "csv/writer.hpp"
#include "spdlog/spdlog.h"

#include "PersistenceManager.h"
#include "Submission.h"
#include "sam.h"

using namespace sam;

PersistenceManager::Writer::Writer(const filesystem::path &filename)
    : file_name_(filename) {
  writer = std::make_unique<csv::Writer>(file_name_);
  writer->configure_dialect().delimiter(",");
}

PersistenceManager::Writer::Writer(const filesystem::path &filename, const std::vector<std::string> colnames)
    : file_name_(filename), column_names(colnames) {
      writer = std::make_unique<csv::Writer>(file_name_);
      writer->configure_dialect().delimiter(",")
                                 .column_names(column_names);
      
      is_header_set = true;
}

PersistenceManager::Writer::~Writer() {
  spdlog::info("Saved {}", file_name_);
  writer->close();
}

void PersistenceManager::Writer::write(const std::map<std::string, std::string> &row) {
  writer->write_row(row);
}

void PersistenceManager::Writer::write(const std::vector<std::string> &row_entries) {
  writer->write_row(row_entries);
}

void PersistenceManager::Writer::write(std::vector<Submission> &subs, int simid) {

  int i = 0;

  /// @todo This looks strange, and it's also very inefficient! Optimize it!
  /// It's somewhat better than &&s, but not great yet!
  for (auto &s : subs) {
    if (!is_header_set) {
      writer->configure_dialect().column_names(Submission::Columns());
      is_header_set = true;
    }

    std::map<std::string, std::string> record{s};
    record["pubid"] = std::to_string(i++);

    writer->write_row(record);
  }

  spdlog::trace("Publications: {}", subs);
}

void PersistenceManager::Writer::setColumnNames(const std::vector<std::string> &colnames) {
  writer->configure_dialect().column_names(colnames);
}

PersistenceManager::Reader::Reader(const filesystem::path &filename)
    : file_name_(filename) {
  reader = std::make_unique<csv::Reader>();
  reader->configure_dialect().delimiter(",").header(false);
  reader->read(file_name_);
};

PersistenceManager::Reader::~Reader() {}

void PersistenceManager::Writer::write(std::vector<arma::Row<float>> &data,
                                       int sid) {

  static std::vector<std::string> cols(data.size());
  static std::map<std::string, std::string> row;

  if (!is_header_set) {
    std::generate(cols.begin(), cols.end(),
                  [n = 0]() mutable { return "g" + std::to_string(n++); });
    cols.push_back("simid");
    cols.push_back("inx");
    writer->configure_dialect().column_names(cols);
    is_header_set = true;
  }

  for (int j = 0; j < data[0].size(); ++j) {
    for (int i = 0; i < data.size(); ++i) {
      row[cols[i]] = std::to_string(data[i][j]);
    }
    row["simid"] = std::to_string(sid);
    row["inx"] = std::to_string(j);

    writer->write_row(row);
  }
}

void PersistenceManager::Writer::write(Experiment *experiment, string_view mode,
                                       int sid) {

  static std::vector<std::string> cols;
  static std::map<std::string, std::string> row;

  if (mode == "full") {

  } else if (mode == "stats") {
    if (!is_header_set) {
      cols.push_back("simid");
      cols.push_back("inx");
      cols.push_back("group");
      cols.push_back("dv");
      cols.push_back("onobs");
      cols.push_back("nobs");
      cols.push_back("means");
      cols.push_back("vars");
      cols.push_back("stddev");
      cols.push_back("sei");
      cols.push_back("statistics");
      cols.push_back("pvalues");
      cols.push_back("effects");
      writer->configure_dialect().column_names(cols);
      is_header_set = true;
    }

    // Note: At the moment I don't write the control group data back to the file
    row["simid"] = std::to_string(sid);
    row["inx"] = std::to_string(++counter);
    for (int g{experiment->setup.nd()},
         d{0}; // Skipping the first group entirely
         g < experiment->setup.ng(); ++g, d %= experiment->setup.nd()) {
      row["group"] = std::to_string(g);
      row["dv"] = std::to_string(d);
      row["onobs"] = std::to_string(experiment->setup.nobs()[g]);
      row["nobs"] = std::to_string((*experiment)[g].nobs_);
      row["means"] = std::to_string((*experiment)[g].mean_);
      row["vars"] = std::to_string((*experiment)[g].var_);
      row["stddev"] = std::to_string((*experiment)[g].stddev_);
      row["sei"] = std::to_string((*experiment)[g].sei_);
      row["statistics"] = std::to_string((*experiment)[g].stats_);
      row["pvalues"] = std::to_string((*experiment)[g].pvalue_);
      row["effects"] = std::to_string((*experiment)[g].effect_);

      writer->write_row(row);
    }
  }
}
