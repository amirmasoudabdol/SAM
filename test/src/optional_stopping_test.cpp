//
// Created by Amir Masoud Abdol on 2019-05-06.
//


#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE OptionalStopping Tests

#include <boost/test/unit_test.hpp>

#include <armadillo>
#include <iostream>
#include <fstream>

#include "HackingStrategy.h"
#include "Researcher.h"

#include "nlohmann/json.hpp"

#include "sam.h"

using namespace arma;
using namespace sam;
using namespace std;

using json = nlohmann::json;


BOOST_AUTO_TEST_CASE( simple_optional_stopping )
{


}