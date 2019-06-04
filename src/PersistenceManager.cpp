//
// Created by Amir Masoud Abdol on 2019-06-03.
//

#include <iostream>

#include "csv/reader.hpp"
#include "csv/writer.hpp"

#include "sam.h"
#include "Submission.h"
#include "PersistenceManager.h"

using namespace sam;

PersistenceManager::Writer::Writer(string filename) : filename(filename) {
    writer = std::make_unique<csv::Writer>(filename);
    writer->configure_dialect().delimiter(", ");
    
};

PersistenceManager::Writer::~Writer() {
   writer->close();
}

void PersistenceManager::Writer::write(std::vector<Submission> &subs, int sid) {
    
    int i = 0;
    // This initiates a copy and it's not very efficient.
    // TODO: Optimize me!
    for (std::map<std::string, std::string> &&s : subs) {
        if (!is_header_set) {
            writer->configure_dialect().column_names(subs.front().cols());
            is_header_set = true;
        }

        s["simid"] = std::to_string(sid);
        s["pubid"] = std::to_string(i++);

        writer->write_row(s);
    }

    // Fix me! 
    if (FLAGS::VERBOSE) {
        for (auto &s : subs) {
            std::cout << s << std::endl;
        }
    }
}

void PersistenceManager::Writer::write(std::vector<arma::Row<double> > measurements) {
    // Do stuff
}

