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
#include "DecisionStrategy.h"

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
                                        .createDecisionStrategy(de_s_conf)
                                        .createJournal(j_conf)
                                        .addNewHackingStrategy(hs)
                                        .build();

    BOOST_TEST((new_researcher.decision_strategy->selectionPref == DecisionPreference::PreRegisteredOutcome));

    // Aha! This is where the open-close principle shows itself. I cannot access
    // alpha here because SelectionStrategy abstract class doesn't have a `params`
    // instance, and even if it had, it would not be the same as the one that I
    // created via the factory.
//    BOOST_TEST((new_researcher.journal->selection_strategy->params.alpha == 0.05));


}

BOOST_AUTO_TEST_SUITE_END()
