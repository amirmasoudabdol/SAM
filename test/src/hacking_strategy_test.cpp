//
// Created by Amir Masoud Abdol on 2019-05-03.
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE HackingStrategy Tests

#include <boost/test/unit_test.hpp>
namespace tt = boost::test_tools;

#include <armadillo>
#include <iostream>
#include <vector>
#include <algorithm>


#include "sam.h"
#include "Experiment.h"
#include "ExperimentSetup.h"

using namespace arma;
using namespace sam;
using namespace std;

BOOST_AUTO_TEST_SUITE( optional_stopping );

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE( outliers_removal );

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE( groups_pooling );

BOOST_AUTO_TEST_SUITE_END();