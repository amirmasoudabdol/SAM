//
// Created by Amir Masoud Abdol on 2019-05-07.
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE Researcher Tests

#include <boost/test/unit_test.hpp>

#include <armadillo>
#include <iostream>
#include <fstream>

#include "HackingStrategy.h"
#include "Researcher.h"

#include "sam.h"

using namespace arma;
using namespace sam;
using namespace std;

using json = nlohmann::json;

BOOST_AUTO_TEST_CASE( honest_researcher )
{

    std::ifstream configfile("../config_file.json");
    json config;
    configfile >> config;




}