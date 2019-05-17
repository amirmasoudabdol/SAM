//
// Created by Amir Masoud Abdol on 2019-05-16.
//

#ifndef SAMPP_IOMANAGER_H
#define SAMPP_IOMANAGER_H


#include <map>
#include <vector>
#include <string>
#include <iostream>

#include <csv/reader.hpp>
#include <csv/writer.hpp>


#include "sam.h"

namespace sam {

    using namespace std;

    class IOManager {

        string filename;
        char mode;

        csv::Reader reader;
        csv::Writer writer;
        
        bool is_header_set = false;

    public:

        IOManager(string filename, mode) : filename(filename), mode(mode) {
            writer = csv::Writer(filename);
            writer.configure_dialect()
                  .delimiter(", ");
            
        };

        void writerSubmissionList(std::vector<Submission> subs) {
            if (is_header_set) {
                writer.configure_dialect()
                      .column_names(subs.front().header());
            }
            
            for (auto &s : subs) {
                writer.write(s);
            }
        }
    };

}

#endif //SAMPP_IOMANAGER_H
