//
// Created by Amir Masoud Abdol on 2019-02-04.
//

#include <ioUtilities.h>

json docoptToJSON(std::map<std::string, docopt::value> args) {
    return json();
}

json readJSON(const std::string& filename) {
    json config;
    std::ifstream configFile(filename);
    configFile >> config;

    return config;
}

void printDocoptArgs(std::map<std::string, docopt::value>& docConfig) {
    for(auto const& arg : docConfig) {
        std::cout << arg.first << ": " << arg.second << std::endl;
    }
}

bool validateJSON(json& config){
    
    
    return true;
}


void validateAndPrepareJSON(json& config){
    
    int ng = config["--n-conditions"].get<int>() * config["--n-dep-vars"].get<int>();
    
    if (config["--means"].is_array()){
        if (config["--means"].size() != ng){
            // Return an error
        }
    }else if (config["--means"].is_number()){
        std::vector<double> means(ng);
        std::fill(means.begin(), means.end(), config["--means"].get<double>());
        config["--means-vector"] = means;
    }
    
    if (config["--sds"].is_array()){
        if (config["--sds"].size() != ng){
            // Return an error
        }
    }else if (config["--sds"].is_number()){
        std::vector<double> sds(ng);
        std::fill(sds.begin(), sds.end(), config["--sds"].get<double>());
        config["--sds-vector"] = sds;
    }
    
    if (config["--cov"].is_array()){
        if (config["--sds"].size() != ng){
            // Return an error
        }
    }else if (config["--sds"].is_number()){
        std::vector<double> sds(ng);
        std::fill(sds.begin(), sds.end(), config["--sds"].get<double>());
        config["--sds-vector"] = sds;
    }
    
    

    
}
