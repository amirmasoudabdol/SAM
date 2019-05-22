//
// Created by Amir Masoud Abdol on 2019-05-22.
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE Journal Tests

#include <boost/test/unit_test.hpp>
namespace tt = boost::test_tools;

#include <armadillo>
#include <iostream>
#include <vector>
#include <algorithm>

#include "sam.h"
#include "test_fixtures.h"

using namespace arma;
using namespace sam;
using namespace std;

bool FLAGS::VERBOSE = false;
bool FLAGS::PROGRESS = false;
bool FLAGS::DEBUG = false;
bool FLAGS::UPDATECONFIG = false;

BOOST_AUTO_TEST_SUITE( constructors )

    BOOST_AUTO_TEST_CASE ( simple_journal )
    {

    }

BOOST_AUTO_TEST_SUITE_END();