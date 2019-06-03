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

namespace sam {

    using namespace std;

    class PersistenceManager {
        
    public:
        class Writer;
        class Reader;
        
        ~PersistenceManager() { };
    };
    
    class PersistenceManager::Writer {
        
        string filename;
        
        std::unique_ptr<csv::Writer> writer;
        
        bool is_header_set = false;
        
    public:
        
        Writer() = default;
        ~Writer();
        
        Writer(string filename);
        
        void write(std::vector<Submission> &subs);
        
        void write(std::vector<arma::Row<double> > measurements);

    };
        
        
    class Reader {
            
    };

}

#endif //SAMPP_PERSISTENCEMANAGER_H
