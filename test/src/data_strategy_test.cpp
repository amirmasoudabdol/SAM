//
// Created by Amir Masoud Abdol on 2019-01-27.
//

#define BOOST_TEST_MODULE DataStrategy Test
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "sam.h"

// most frequently you implement test cases as a free functions with automatic registration
BOOST_AUTO_TEST_CASE( test1 )
{
    // reports 'error in "test1": test 2 == 1 failed'
    BOOST_CHECK( 1 == 1 );
}
