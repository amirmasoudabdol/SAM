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

#include "Researcher.h"
#include "HackingStrategy.h"

using namespace arma;
using namespace sam;
using namespace std;

bool FLAGS::VERBOSE = false;
bool FLAGS::PROGRESS = false;
bool FLAGS::DEBUG = false;
bool FLAGS::UPDATECONFIG = false;

BOOST_AUTO_TEST_SUITE( constructors );

BOOST_AUTO_TEST_SUITE_END();

BOOST_FIXTURE_TEST_SUITE( selection_strategies, SampleResearch );

    BOOST_AUTO_TEST_CASE( significant_selection_pub_bias_rate )
    {

        // Setting up the Research
        
//        nobs = 20;
        mean = 1;
        var = 1;
        nobs = single_sample_find_df(0, mean, var, 0.05, TestStrategy::TestSide::TwoSided);
//        nobs /= 2;
        
        nsims = 10000;
        
        double n_pubs = jp.max_pubs * nsims;

    	initResearch();
        
        // Starting the Test
        
        vec pub_sigs(n_pubs);

    	BOOST_TEST (setup.ng() == ng);
    	
        for (auto &p : {0., 0.05, 0.5, 0.95} ) {
            
            ssp.pub_bias = p;
            researcher.journal->selection_strategy = SelectionStrategy::build(ssp);

            for (int i = 0; i < nsims; i++) {

                while (researcher.journal->isStillAccepting()) {

                    researcher.prepareResearch();

                    researcher.performResearch();

                    researcher.publishResearch();
                    
                }
                
                for (int k = 0; k < jp.max_pubs; ++k)
                {
                    pub_sigs[i * jp.max_pubs + k] = researcher.journal->publications_list[k].sig;
                }
                
                researcher.journal->clear();
            }
            
            std::cout << arma::mean(pub_sigs) << std::endl;
            
            BOOST_TEST( arma::mean(pub_sigs) > ssp.pub_bias );
            
        }

    }

    BOOST_AUTO_TEST_CASE( random_selection )
    {
        
        // Setting up the Research
        
        mean = 0.25;
        var = 1;
        nobs = single_sample_find_df(0, mean, var, 0.05, TestStrategy::TestSide::TwoSided);
        nobs /= 3;

        nsims = 1000;

        double n_pubs = jp.max_pubs * nsims;
        
        ssp.name = SelectionMethod::RandomSelection;

        initResearch();

        // Starting the Test

        vec pub_sigs;

        BOOST_TEST (setup.ng() == ng);

        pub_sigs.set_size(n_pubs);

        for (int i = 0; i < nsims; i++) {

            while (researcher.journal->isStillAccepting()) {

                researcher.prepareResearch();

                researcher.performResearch();

                researcher.publishResearch();

            }

            for (int k = 0; k < jp.max_pubs; ++k)
            {
                pub_sigs[i * jp.max_pubs + k] = researcher.journal->publications_list[k].sig;
            }

            researcher.journal->clear();
        }

        BOOST_CHECK_SMALL( arma::mean(pub_sigs) - 0.5 , 0.1);

        
    }

BOOST_AUTO_TEST_SUITE_END();


