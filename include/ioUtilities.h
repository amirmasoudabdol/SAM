//
// Created by Amir Masoud Abdol on 2019_02; 
//

#ifndef SAMPP_IOUTILITIES_H
#define SAMPP_IOUTILITIES_H

#include <fstream>
#include <iostream>
#include <string>
#include "docopt.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

struct InputParameters {
    
    // SAM
    bool debug = false;
    bool verbose = false;
    bool progress = false;
    
    
    
    int nsims;
    int nconditions;
    int ndependentvars;
    int nitems;
    
    int nobs;
    
    
    // Journal
    int alpha;
//    SelectionType selectionType;
    
    
};

bool validateCliParams(std::map<std::string, docopt::value> docConfig);
bool validateJSON(json jSimConfig);

json readJSON(const std::string& filename);
json docoptToJSON(std::map<std::string, docopt::value>& args);

json mergeCLIAndJSON(std::map<std::string, docopt::value> docConfig, json jSimConfig);

void printDocoptArgs(std::map<std::string, docopt::value>& docConfig);

std::string generateOutputFilename(std::string output_prefix);

#endif //SAMPP_IOUTILITIES_H
