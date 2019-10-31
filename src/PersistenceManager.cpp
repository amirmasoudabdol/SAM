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

PersistenceManager::Writer::Writer(const string &filename) : file_name_(filename) {
    writer = std::make_unique<csv::Writer>(file_name_);
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

    // TODO: Fix me! I'm fine but it's be nicer if there is a more elegant way to do this.
    if (FLAGS::VERBOSE) {
        for (auto &s : subs) {
            std::cout << s << std::endl;
        }
    }
}

PersistenceManager::Reader::Reader(const string &filename) : file_name_(filename) {
    reader = std::make_unique<csv::Reader>();
    reader->configure_dialect().delimiter(",").header(false);
    reader->read(file_name_);
};

PersistenceManager::Reader::~Reader() {
    
}


void PersistenceManager::Reader::read_raw_data(Experiment *expr) {
    
    auto rows = reader->rows();
    
    
    if (expr->setup.ng() != reader->cols().size())
        throw std::length_error("Number of columns in the CSV file doesn't match \
                                the experiment size.");
    
    
    for (int r = 0; r < rows.size(); r++) {
        for (auto &col : reader->cols()) {
            expr->measurements[std::stoi(col)][r] = std::stod(rows[r][col]);
        }
    }
    
}

void PersistenceManager::Writer::write(std::vector<arma::Row<double> > &data, int sid) {
    
    static std::vector<std::string> cols(data.size());
    static std::map<std::string, std::string> row;
    
    if (!is_header_set){
        std::generate(cols.begin(), cols.end(), [n = 0]() mutable {return "g" + std::to_string(n++) ; });
        cols.push_back("simid");
        cols.push_back("inx");
        writer->configure_dialect().column_names(cols);
        is_header_set = true;
    }
    
    for (int j = 0; j < data[0].size(); ++j){
        for (int i = 0; i < data.size(); ++i) {
            row[cols[i]] = std::to_string(data[i][j]);
        }
        row["simid"] = std::to_string(sid);
        row["inx"] = std::to_string(j);
        
        writer->write_row(row);
    }
    
}


void PersistenceManager::Writer::write(Experiment *experiment, string_view mode, int sid) {
    
    static std::vector<std::string> cols;
    static std::map<std::string, std::string> row;
    
    if (mode == "full") {
        
    }else if (mode == "stats"){
        if(!is_header_set){
            cols.push_back("simid");
            cols.push_back("inx");
            cols.push_back("group");
            cols.push_back("dv");
            cols.push_back("nobs");
            cols.push_back("means");
            cols.push_back("vars");
            cols.push_back("stddev");
            cols.push_back("ses");
            cols.push_back("statistics");
            cols.push_back("pvalues");
            cols.push_back("effects");
            writer->configure_dialect().column_names(cols);
            is_header_set = true;
        }
        
        // Note: At the moment I don't write the control group data back to the file
        row["simid"] = std::to_string(sid);
        row["inx"] = std::to_string(++counter);
        for (int g{experiment->setup.nd()}, d{0};
                g < experiment->setup.ng();
                ++g, d%=experiment->setup.ng()) {
            row["group"] = std::to_string(g);
            row["dv"] = std::to_string(d);
            row["nobs"] = std::to_string(experiment->nobs[g]);
            row["means"] = std::to_string(experiment->means[g]);
            row["vars"] = std::to_string(experiment->vars[g]);
            row["stddev"] = std::to_string(experiment->stddev[g]);
            row["ses"] = std::to_string(experiment->ses[g]);
            row["statistics"] = std::to_string(experiment->statistics[g]);
            row["pvalues"] = std::to_string(experiment->pvalues[g]);
            row["effects"] = std::to_string(experiment->effects[g]);
                        
            writer->write_row(row);
        }
        
        
    }
}

