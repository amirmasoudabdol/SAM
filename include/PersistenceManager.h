//
// Created by Amir Masoud Abdol on 2019-05-16.
//

#ifndef SAMPP_PERSISTENCEMANAGER_H
#define SAMPP_PERSISTENCEMANAGER_H


#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <memory>

#include <csv/reader.hpp>
#include <csv/writer.hpp>

#include "sam.h"
#include "Submission.h"
#include "Experiment.h"

namespace sam {

    using namespace std;

    class PersistenceManager {
        
    public:
        class Writer;
        class Reader;
        
        ~PersistenceManager() { };
    };
    
    class PersistenceManager::Writer {
        
        string file_name_;
        int counter {0};
        
        std::unique_ptr<csv::Writer> writer;
        
        bool is_header_set = false;
        
    public:
        
        Writer() = default;
        ~Writer();
        
        Writer(const string &filename);
        
        /// Write a list of submission records to a file, or a database
        /// @param subs A reference to Submission container
        void write(std::vector<Submission> &subs, int sid = 0);
        
        /// Write each groups' data to a file, or a database
        /// @param data A reference to the Experiment->measurements
        void write(std::vector<arma::Row<double>> &data, int sid = 0);
        
        
        /// Write part of the Experiment to a file, or a database
        /// @param A constance reference to the Experiment
        void write(Experiment* experiment, string_view mode, int sid);
        
    };
        
        
    class PersistenceManager::Reader {
        
        string file_name_;
        std::unique_ptr<csv::Reader> reader;
        
    public:
        Reader() = default;
        ~Reader();
        
        Reader(const string &filename);
        
        void filename(const string &name) {
            file_name_ = name;
            reader->read(file_name_);
        };
        
        void read_raw_data(Experiment *epxr);
            
    };

}

#endif //SAMPP_PERSISTENCEMANAGER_H
