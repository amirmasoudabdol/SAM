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

namespace sam {

    using namespace std;

    class PersistenceManager {
        
    public:
        class Writer;
        class Reader;
    };
    
    class PersistenceManager::Writer {
        
        string filename;
        
        std::unique_ptr<csv::Writer> writer;
        
        bool is_header_set = false;
        
    public:
        
        Writer(string filename) : filename(filename) {
            writer = std::make_unique<csv::Writer>(filename);
            writer->configure_dialect()
            .delimiter(", ")
            .column_names("simid", "pubid", "inx", "nobs", "yi", "vi", "sei", "statistic", "pvalue", "effect", "sig", "side", "isHacked", "tnobs");
            
        };
        
        ~PersistenceManager() {
            writer->close();
        }
        
        void write(std::vector<Submission> subs) {
             if (is_header_set) {
                 writer->configure_dialect()
                       .column_names(subs.front().col_names());
             }
            
            for (std::unordered_map<std::string, std::string> &&s : subs) {
                writer->write_row(s);
            }
        }
        
        void write(std::vector<arma::Row<double> > measurements) {
            // Do stuff
        }

    }

}

#endif //SAMPP_PERSISTENCEMANAGER_H
