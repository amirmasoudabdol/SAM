//
// Created by Amir Masoud Abdol on 2019-05-16.
//

#ifndef SAMPP_PERSISTENCEMANAGER_H
#define SAMPP_PERSISTENCEMANAGER_H

#include <csv/reader.hpp>
#include <csv/writer.hpp>

#include "Experiment.h"
#include "sam.h"

namespace sam {

using namespace std;

/// Persistence Manager class, dealing with CSV Read/Write.
/// This is designed such that I can hopefully replace the CSV with
/// something more efficient later.
class PersistenceManager {

public:
  class Writer;
  class Reader;

  ~PersistenceManager() = default;
};

/// Declaration of the Writer class
class PersistenceManager::Writer {

  //! Filename
  string path_;
  string prefix_;
  string file_name_;

  //! Keeps track of number of records currently written into the file
  int counter{0};

  //! A unique pointer to a new writer object,
  //! @todo: Check if there is a better way of implementing this
  std::unique_ptr<csv::Writer> writer;
  
  std::vector<std::string> column_names;

  bool is_header_set = false;

public:
  Writer() = default;
  ~Writer();

  Writer(const std::filesystem::path &filename);
  
  Writer(const std::filesystem::path &path, const string &prefix, const std::filesystem::path filename);
  
  Writer(const std::filesystem::path &filename, const std::vector<std::string> colnames);
  
  void write(const std::vector<std::string> &row_entries);
  
  void write(const std::map<string, string> &row);

  void write(const Submission &sub);

  /// Write a list of submission records to a file, or a database
  /// @param subs A reference to Submission container
  void write(std::vector<Submission> &subs, int sid = 0);

  /// Write each groups' data to a file, or a database
  /// @param data A reference to the Experiment->measurements
  void write(std::vector<arma::Row<float>> &data, int sid = 0);

  /// Write part of the Experiment to a file, or a database
  /// @param A constance reference to the Experiment
  void write(Experiment *experiment, string_view mode, int sid);
  
  void setColumnNames(const std::vector<std::string> &colnames);
  
};

class PersistenceManager::Reader {

  string file_name_;
  std::unique_ptr<csv::Reader> reader;

public:
  Reader() = default;
  ~Reader();

  Reader(const std::filesystem::path &filename);

  void filename(const std::filesystem::path &name) {
    file_name_ = name;
    reader->read(file_name_);
  };

  void read_raw_data(Experiment *expr);
};

} // namespace sam

#endif // SAMPP_PERSISTENCEMANAGER_H
