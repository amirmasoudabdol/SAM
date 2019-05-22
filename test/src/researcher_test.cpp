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

using namespace arma;
using namespace sam;
using namespace std;

using json = nlohmann::json;

bool FLAGS::VERBOSE = false;
bool FLAGS::PROGRESS = false;
bool FLAGS::DEBUG = false;
bool FLAGS::UPDATECONFIG = false;


struct sample_researcher {

//    BOOST_TEST_MESSAGE( "Setup a sample ExperimentSetup and ExperimentSetup" );

    int nc = 2;
    int nd = 3;
    int ni = 0;
    int ng = nc * nd;

    int nobs = 25;
    double mean = 0.25;
    double var = 1;
    double cov = 0.01;

    arma::Row<int> v_nobs;
    arma::Row<double> v_means;
    arma::Row<double> v_vars;
    arma::Mat<double> v_sigma;

    DataStrategy::DataStrategyParameters dsp;

    TestStrategy::TestStrategyParameters tsp;

    EffectStrategy::EffectStrategyParameters esp;

    string ds_name = "LinearModel";

    ExperimentSetup setup;

    Experiment *experiment;

    Journal *journal;
    Journal::JournalParameters jp;

    SelectionStrategy *ss;
    SelectionStrategy::SelectionStrategyParameters ssp;

    DecisionStrategy *ds;
    DecisionStrategy::DecisionStrategyParameters desp;

    HackingStrategy *hs;
    HackingStrategy::HackingStrategyParameters hsp;

    sample_researcher() {
        v_nobs = arma::Row<int>(ng).fill(nobs);
        v_means = arma::Row<double>(ng).fill(mean);
        v_vars = arma::Row<double>(ng).fill(var);
        v_sigma = arma::Mat<double>(ng, ng).fill(cov);
        v_sigma.diag() = v_vars;

        dsp.name = ds_name;

        tsp.name = TestStrategy::TestMethod::TTest;
        tsp.side = TestStrategy::TestSide::TwoSided;
        tsp.alpha = 0.05;

        esp.name = "CohensD";

        setup = ExperimentSetup::create()
                .setNumConditions(nc)
                .setNumDependentVariables(nd)
                .setNumItems(ni)
                .setNumObservations(nobs)
                .setMeans(mean)
                .setVariance(var)
                .setCovariance(cov)
                .setDataStrategy(dsp)
                .setTestStrategy(tsp)
                .setEffectStrategy(esp)
                .build();

        experiment = new Experiment(setup);

        hsp.name = "NoHack";

        desp.name = DecisionType::HonestDecisionMaker;
        desp.preference = DecisionPreference::PreRegisteredOutcome;

        ssp.name = SelectionType::RandomSelection;
        ssp.alpha = 0.05;
        ssp.pub_bias = 0.95;
        ssp.side = 1;
        ssp.seed = 42;

    }

};

BOOST_FIXTURE_TEST_SUITE ( researcher, sample_researcher )

BOOST_AUTO_TEST_CASE( researcher_builder )
{

    Researcher researcher = Researcher::create("John")
                                        .createExperiment(setup)
                                        .createDecisionStrategy(desp)
                                        .createJournal(jp, ssp)
                                        .addNewHackingStrategy(hs)
                                        .build();

    BOOST_TEST((researcher.decision_strategy->selectionPref == DecisionPreference::PreRegisteredOutcome));

    BOOST_TEST((researcher.journal->selection_strategy->params.alpha == 0.05));


}

BOOST_AUTO_TEST_SUITE_END()