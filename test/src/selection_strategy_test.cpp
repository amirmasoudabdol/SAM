//
// Created by Amir Masoud Abdol on 2019-05-22.
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE SelectionStrategy Tests

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

BOOST_AUTO_TEST_SUITE( constructors );

BOOST_AUTO_TEST_SUITE_END();

BOOST_FIXTURE_TEST_SUITE( selection_strategies, sample_journal );

    BOOST_AUTO_TEST_CASE( publication_bias )
    {

    }

BOOST_AUTO_TEST_SUITE_END();


