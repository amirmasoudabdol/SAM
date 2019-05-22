//
// Created by Amir Masoud Abdol on 2019-05-16.
//

#ifndef SAMPP_IOMANAGER_H
#define SAMPP_IOMANAGER_H


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

    class IOManager {

        string filename;
        char mode;

//        csv::Reader reader;
        std::unique_ptr<csv::Writer> writer;
        
        bool is_header_set = false;

    public:

        IOManager(string filename, char mode) : filename(filename), mode(mode) {
            writer = std::make_unique<csv::Writer>(filename);
            writer->configure_dialect()
                  .delimiter(", ")
                  .column_names("simid", "pubid", "inx", "nobs", "yi", "vi", "sei", "statistic", "pvalue", "effect", "sig", "side", "isHacked", "tnobs");
            
        };

        ~IOManager() {
            writer->close();
        }

        void writerSubmissionList(std::vector<Submission> subs) {
            // if (is_header_set) {
            //     writer->configure_dialect()
            //           .column_names(subs.front().col_names());
            // }
            
            for (std::unordered_map<std::string, std::string> &&s : subs) {
                writer->write_row(s);
            }
        }
    };

}

#endif //SAMPP_IOMANAGER_H
