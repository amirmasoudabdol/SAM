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
