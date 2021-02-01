//
// Created by Amir Masoud Abdol on 2019-05-07.
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE Researcher Tests

#include <boost/test/unit_test.hpp>

#include <armadillo>
#include <iostream>
#include <fstream>

#include "ExperimentSetup.h"
#include "Experiment.h"
#include "Researcher.h"
#include "HackingStrategy.h"
#include "Journal.h"
#include "ResearchStrategy.h"

#include "sam.h"

#include "test_fixtures.h"

using namespace arma;
using namespace sam;
using namespace std;

using json = nlohmann::json;

bool FLAGS::VERBOSE = false;
bool FLAGS::PROGRESS = false;
bool FLAGS::DEBUG = false;
bool FLAGS::UPDATECONFIG = false;

BOOST_AUTO_TEST_CASE ( default_constructor )
{
    Researcher researcher;
}

BOOST_FIXTURE_TEST_SUITE ( researcher, SampleResearch )

BOOST_AUTO_TEST_CASE( researcher_builder )
{
    
    initResearch();

    Researcher new_researcher = Researcher::create("John")
                                        .createExperiment(setup)
                                        .createResearchStrategy(de_s_conf)
                                        .createJournal(j_conf)
                                        .addNewHackingStrategy(hs)
                                        .build();

    // Aha! This is where the open-close principle shows itself. I cannot access
    // alpha here because ReviewStrategy abstract class doesn't have a `params`
    // instance, and even if it had, it would not be the same as the one that I
    // created via the factory.
//    BOOST_TEST((new_researcher.journal->review_strategy->params.alpha == 0.05));

        // The same here as well, I've removed the params from the Abstract class and this is the result.
        // BOOST_TEST((new_researcher.research_strategy->params.preference == DecisionPreference::PreRegisteredOutcome));
}

BOOST_AUTO_TEST_SUITE_END()
