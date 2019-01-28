#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include "Utilities.h"
#include "docopt.h"
#include <nlohmann/json.hpp>

#include <Experiment.h>
#include <ExperimentSetup.h>
#include <EffectEstimators.h>
#include <DataGenStrategy.h>
#include <TestStrategies.h>
#include <RandomNumberGenerator.h>
#include <Researcher.h>
#include <Journal.h>
#include <SelectionStrategies.h>

using json = nlohmann::json;

json inputParams;

static const char USAGE[] =
R"(SAMpp

    Usage:
        SAMpp [--master-seed S] [--config FILE]
              [--n-conditions C] [--n-dvs D]
              [--n-obs N] [--means M] [--vars V]
              [--is-correlated] [--cov-const CV]
              [--alpha A] [--pub-bias B] [--max-pubs K]
              [--is-p-hacker] [--hacking-methods-config JSON]
              [--output-path PATH] [--output-prefix PREFIX]



    Options:
        -h --help          Show this screen.
        -v --version          Show version.
        --verbose          Print more texts.
        --master-seed=S    Set the master seed [default: 42]
        --n-sims=I         Number of simulations [default: 10000]
        --n-conditions=C   Number of conditions [default: 1]
        --n-dvs=D          Number of dependent variables [default: 1]
        --alpha=A          Alpha [default: 0.05]
        --pub-bias=B       Publication bias rate [default: 0.95]
        --max-pubs=K       Maximum number of publications [default: 70]
        --n-obs=N          Number of observations [default: 20]
        --means=M            List of means for each group [default: 0.15]
        --vars=V             List of variances for each group [default: 0.01]
        --is-correlated     Whether conditions are correlated [default: false]
        --cov-const=CV        Constant covariant [default: 0.5]
        --output-prefix=PREFIX    Output prefix used for saving files [default: auto]
        --output-path=PATH      Output path [default: output/]
        --config=FILE      JSON config file [default: ../sample_config.json]
        --is-p-hacker      If true, the Researcher will perform phacking techniques on the data [default: false]
        --hacking-methods-config=FILE  JSON config storing p-hacking methods and their parameters [default: ../sample_hacking_methods.json]
)";

// for convenience
void testRandom();
void estherSimulation();
void testTTest();
void effectEstimatorTest();
void dataGenStrategyTest();
void testRandomClass();
void testJSON(std::string file);
void testDOCOPT(std::map<std::string, docopt::value> args);

using json = nlohmann::json;


// TODO: This works for now but it needs more casting to be a better interface between JSON and DOCOPT
namespace nlohmann {
    template <>
    struct adl_serializer<docopt::value> {
        static void to_json(json& j, docopt::value t) {
            if (t.isString()) {
                j = t.asString();
            }else if (t.isBool()){
                j = t.asBool();
            }else if (t.isLong()){
                j = t.asLong();
            }
        }
    };
}


int main(int argc, const char** argv){

    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
                                                               { argv + 1, argv + argc },
                                                               true,                        // show help if requested
                                                               "SAMpp 0.1 (Alpha)");        // version string

//    for(auto const& arg : args) {
//        std::cout << arg.first << ": " << arg.second << std::endl;
//    }

//    testDOCOPT(args);
//    testJSON(args["--config"].asString());
//    testJSON(args["--hacking-methods-config"].asString());

//    testRandom();
//    testTTest();

    estherSimulation();
//    effectEstimatorTest();
//    testRandomClass();

    return 0;
}

void testDOCOPT(std::map<std::string, docopt::value> args){
    json config;

    for(auto const& arg : args) {
//        if (arg.second.isString())
            config[arg.first] = arg.second;
//        std::cout << arg.first << ": " << arg.second << std::endl;
    }

    std::cout << config;

}

void testJSON(std::string file){

    // TODO: Check if I'm getting a vector or constant, and change accordingly

    std::cout << "\ntestJSON()\n\n";

    json config;
    std::ifstream configFile(file);
    configFile >> config;

//    for (auto& item : config){
//    for (auto item : config.items()){
//        std::cout << item.key()
//        << ": " << item.value() << std::endl;
//    }

    std::cout << config.dump(4);

//    std::vector<std::vector<double> > cov_matrix = config["--cov-matrix"];
//    for (int i = 0; i < 3; ++i) {
//        for (int j = 0; j < 3; ++j) {
//            std::cout << i << ", " << j << ", :" << cov_matrix[i][j] << std::endl;
//        }
//    }
}

void testRandom() {
    std::__1::cout << "Hello" << std::__1::endl;
    std::__1::cout << "Here are some random numbers..." << std::__1::endl;
//    std::vector<std::vector<double>> rng = norm_rng();
//    std::vector<std::vector<double>> rng = gen_norm_rng();

    std::__1::vector<std::__1::vector<double>> rng = mvnorm_rng();

    for (auto &row : rng){
//        for (auto &v : row){
//            std::cout << v << ',';
//        }
        std::__1::cout << "mean: " << mean(row);
        std::__1::cout << " - sd: " << standard_dv(row);
        std::__1::cout << std::__1::endl;
    }

    std::__1::cout << "cor: " << cor(rng, 0, 1);
}

void testTTest(){
    auto ttest_res = oneSampleTTest(15, sqrt(87.5), 13, 12, sqrt(39), 11, true);
    std::cout << "t: " << ttest_res.first << "p: " << ttest_res.second << std::endl;
}

void effectEstimatorTest(){
    StandardMeanDiffEffectEstimator _effect_estimator;
    _effect_estimator.computeEffectFromStats(1, 2, 3, 4, 5, 6);
}


void estherSimulation(){

    // Declaring sample paramters
    int nc = 1;
    int nd = 4;
    int nobs = 200;
    std::vector<double> means {.15, .15, 0.147, 0.147};
    std::vector<double> vars {0.01, 0.01, 0.01, 0.01};

    double alpha= 0.95;
    double pub_bias = 0.95;


    RandomNumberGenerator rngEngine(42, false);

    ExperimentSetup estherSetup(nc, nd, nobs, means, vars);

    Experiment estherExperiment(estherSetup);

    FixedEffectStrategy fixedEffectModel(estherSetup, rngEngine);
    estherExperiment.setDataStrategy(&fixedEffectModel);

//    estherExperiment.initExperiment();
    estherExperiment.allocateResources();
    estherExperiment.generateData();
    estherExperiment.calculateStatistics();

    Researcher esther(estherExperiment);

    TTest tTest(estherExperiment);
    esther.setTestStrategy(&tTest);


    esther.calculateEffect();
    esther.runTest();

    Journal journal;
    SignigicantSelection sigSelection(alpha, pub_bias);
    journal.setSelectionStrategy(&sigSelection);
//
    esther.setJournal(journal);
//
    esther.selectTheOutcome();
    esther.prepareTheSubmission();
    esther.submitToJournal();
//
//    std::cout << esther.journal.submissionList[0];




//    for (int i = 0; i < esther.experiment.setup.ng; ++i) {
//        std::cout << esther.experiment.means[i] << ", ";
//        std::cout << esther.experiment.vars[i] << ", ";
//        std::cout << esther.experiment.ses[i] << ", ";
//        std::cout << esther.experiment.statistics[i] << ", ";
//        std::cout << esther.experiment.pvalues[i] << ", ";
//        std::cout << esther.experiment.effects[i] << ", ";
//        std::cout << std::endl;
//    }

}

void testRandomClass(){
    std::cout << std::endl;
    RandomNumberGenerator normal(42, true);

    std::vector<double> means = {10, 20, 30};
    std::vector<std::vector<double >> vars = {{1, .5, 0}, {.5, 1, 0}, {0, 0, 1}};

    auto mvnormrng = normal.mvnorm(means, vars);
    auto rng = normal.mvnorm(means, vars, 100);

//    auto rng = normal.normal({10, 20}, {1, 5}, 10);

//    for (auto &row : rng){
//        std::cout << "mean: " << mean(row);
//        std::cout << " - sd: " << sd(row);
//        std::cout << std::endl;
//    }


}
