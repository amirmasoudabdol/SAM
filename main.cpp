#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include "Utilities.h"
#include "docopt.h"
#include "tqdm/tqdm.h"

#include <Experiment.h>
#include <ExperimentSetup.h>
#include <EffectEstimators.h>
#include <DataGenStrategy.h>
#include <TestStrategy.h>
#include <RandomNumberGenerator.h>
#include <Researcher.h>
#include <Journal.h>
#include <SelectionStrategies.h>
#include <HackingStrategies.h>
#include <DecisionStrategy.h>

#include "ioUtilities.h"

using json = nlohmann::json;

// \cond HIDDEN_SYMBOLS
// TODO: This works for now but it needs more casting to be a better interface between JSON and DOCOPT
//namespace nlohmann {
//    template <>
//    struct adl_serializer<docopt::value> {
//        static void to_json(json& j, docopt::value t) {
//            if (t.isString()) {
//                j = t.asString();
//            }else if (t.isBool()){
//                j = t.asBool();
//            }else if (t.isLong()){
//                j = t.asLong();
//            }
//        }
//    };
//}
// \endcond

json inputParams;

static const char USAGE[] =
R"(SAMpp

    Usage:
        SAMpp [--master-seed S] [--config FILE]
              [--n-conditions C] [--n-dep-vars D]
              [--n-items I]
              [--n-obs O] [--means M] [--sd T]
              [--is-correlated] [--cov-const CV]
              [--alpha A] [--pub-bias B] [--max-pubs K]
              [--is-p-hacker] [--hacking-methods-config JSON]
              [--output-path PATH] [--output-prefix PREFIX]
              [--progress]



    Options:
        -h --help          Show this screen.
        -v --version       Show version.
        --verbose          Print more texts.
        --progress         Show progress bar [default: false]
        --master-seed=S    Set the master seed [default: 42]
        --n-sims=N         Number of simulations [default: 10000]
        --n-conditions=C   Number of conditions [default: 1]
        --n-dep-vars=D     Number of dependent variables [default: 1]
        --n-items=I        Number of items [default: 0]
        --alpha=A          Alpha [default: 0.05]
        --pub-bias=B       Publication bias rate [default: 0.95]
        --max-pubs=K       Maximum number of publications [default: 70]
        --n-obs=O          Number of observations [default: 20]
        --means=M            List of means for each group [default: 0.15]
        --sd=T             List of variances for each group [default: 0.01]
        --is-correlated     Whether conditions are correlated [default: false]
        --cov-const=CV        Constant covariant [default: 0.5]
        --output-prefix=PREFIX    Output prefix used for saving files [default: auto]
        --output-path=PATH      Output path [default: output/]
        --config=FILE      JSON config file [default: /Users/amabdol/Projects/SAMpp/sample_config.json]
        --is-p-hacker      If true, the Researcher will perform phacking techniques on the data [default: false]
        --hacking-methods-config=FILE  JSON config storing p-hacking methods and their parameters [default: ../sample_hacking_methods.json]
)";

// for convenience
void testRandom();
void estherSimulation();
void estherSimulationTest();
void testTTest();
void effectEstimatorTest();
void dataGenStrategyTest();
void testRandomClass();
void testJSON(std::string file);
void testDOCOPT(std::map<std::string, docopt::value> args);
void latentStrategyTest();

void runSimulation(json& simConfig);

using json = nlohmann::json;

tqdm simulationBar;

int main(int argc, const char** argv){

    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
                                                               { argv + 1, argv + argc },
                                                               true,                        // show help if requested
                                                               "SAMpp 0.1 (Alpha)");        // version string


    json jSimConfig = readJSON(args["--config"].asString());

//    testRandom();
//    testTTest();

//    estherSimulation();

    runSimulation(jSimConfig);
    
//    latentStrategyTest();
    
//    estherSimulationTest();
//    effectEstimatorTest();
    
//   testRandomClass();

    return 0;
}

void runSimulation(json& simConfig){

    int masterSeed;
    if (simConfig["--master-seed"] == "random") {
        masterSeed = rand();
    }else{
        masterSeed = simConfig["--master-seed"].get<int>();
    }
    RandomNumberGenerator mainRNGengine(masterSeed, simConfig["--is-multivariate"]);
    std::cout << "Master seed: " << masterSeed << std::endl;
    
//    std::cout << "1\n";
    ExperimentSetup experimentSetup;
    if (simConfig["--data-strategy"] == "FixedModel"){
        if (!simConfig["--is-multivariate"]) {
        //        // Initializing Experiment Setup
           experimentSetup = ExperimentSetup(simConfig["--n-conditions"],
                                           simConfig["--n-dep-vars"],
                                           simConfig["--n-obs"],
                                           simConfig["--means"].get<std::vector<double>>(),
                                           simConfig["--sds"].get<std::vector<double>>());
        }else{
            // Initializing Experiment Setup
            experimentSetup = ExperimentSetup(simConfig["--n-conditions"],
                                            simConfig["--n-dep-vars"],
                                            simConfig["--n-obs"],
                                            simConfig["--means"].get<std::vector<double>>(),
                                            simConfig["--cov-matrix"].get<std::vector<std::vector<double>>>());
        }
    }
    
    if (simConfig["--data-strategy"] == "LatentModel") {
        std::cout << "Latent Model\n";
        experimentSetup = ExperimentSetup(simConfig["--n-conditions"],
                                          simConfig["--n-dep-vars"],
                                          simConfig["--n-items"],
                                          simConfig["--n-obs"],
                                          simConfig["--factor-loadings"].get<std::vector<double>>(),
                                          simConfig["--factor-means"].get<std::vector<double>>(),
                                          simConfig["--factor-cov"].get<std::vector<std::vector<double>>>(),
                                          simConfig["--error-cov"].get<std::vector<std::vector<double>>>());
        std::cout << "Latent Model Setup!\n";
    }

    
    // Initializing Journal
    Journal journal(simConfig["--max-pubs"],
                    simConfig["--pub-bias"],
                    simConfig["--alpha"]);
    SignigicantSelection sigSelection(simConfig["--pub-bias"], simConfig["--alpha"]);
    journal.setSelectionStrategy(&sigSelection);
    std::cout << "Initializing Journal, Done!\n";

    // Initializing Experiment
    Experiment experiment(experimentSetup);
        std::cout << "Initializing Experiment, Done!\n";

        // Setting Data Model
//        FixedEffectStrategy fixedEffectModel;
        FixedEffectStrategy fixedEffectModel(mainRNGengine);
    
//        LatentDataStrategy latentDataModel;
        LatentDataStrategy latentDataModel(mainRNGengine);
        // FIXME: I cannot make these object so nicely, I think I need a factor for a few of them
        if (simConfig["--data-strategy"] == "FixedModel"){
            
            experiment.setDataStrategy(&fixedEffectModel);
        }
        else if (simConfig["--data-strategy"] == "LatentModel") {
//            std::cout << "LATENT MODEL";
            
            experiment.setDataStrategy(&latentDataModel);
        }
        std::cout << "Setting Data Model, Done!\n";
    
    
        // Setting the Test Strategy
        TTest tTest;
        experiment.setTestStrategy(&tTest);
        std::cout << "Setting the Test Strategy, Done!\n";

    // Initializing the Researcher
    Researcher researcher(&experiment);
    std::cout << "Initializing the Researcher, Done!\n";

        // Setting the Selection Preference
        researcher.selectionPref = PreRegisteredOutcome;
        std::cout << "Setting the Selection Preference, Done!\n";

        // Assigning the Journal
        researcher.setJournal(&journal);
        std::cout << "Assigning the Journal, Done!\n";

        // Setting the Decision Strategy
        ImpatientDecisionMaker impatientReporter(0, simConfig["--alpha"], researcher.selectionPref);
        researcher.setDecisionStrategy(&impatientReporter);
        std::cout << "Setting the Decision Strategy, Done!\n";

    // Initializing Hacking Routines
    researcher.isHacker = simConfig["--is-phacker"];
    std::cout << "Initializing Hacking Routines, Done!\n";

    // Registering Hacking Methods
    if (simConfig["--is-phacker"]){
        
        // Overwriting the selection preference, this is technically works as the
        // researcher is performing _Outcome Switching_.
        if (simConfig["--selection-pref"] == "MinPvalue"){
            researcher.selectionPref = MinPvalue;
            researcher.decisionStrategy->selectionPref = MinPvalue;
        } /* else if for other options */

        if (!simConfig["--p-hacking-methods"].is_null()) {
            for (auto &item : simConfig["--p-hacking-methods"]){
                if (item["type"] == "OptionalStopping") {
                    OptionalStopping optStopping(item["size"], item["attemps"]);
                    researcher.registerAHackingStrategy(&optStopping);
                }/*else if for other options*/
                else if(item["type"] == "SDOutlierRemoval") {
                    SDOutlierRemoval cfOutlierRemoval(item["sd_multiplier"]);
                    researcher.registerAHackingStrategy(&cfOutlierRemoval);
                }
            }
        }
        
//        json hackingConfig = readJSON(simConfig["--p-hacking-config-file"]);
//        for (auto &item : hackingConfig["--p-hacking-methods"]){
//            if (item["type"] == "OptionalStopping") {
//                OptionalStopping optStopping(item["size"], item["attemps"]);
//                researcher.registerAHackingStrategy(&optStopping);
//            }/*else if for other options*/
//            else if(item["type"] == "SDOutlierRemoval") {
//                SDOutlierRemoval cfOutlierRemoval(item["sd_multiplier"]);
//                researcher.registerAHackingStrategy(&cfOutlierRemoval);
//            }
//        }
    }
    std::cout << "Registering Hacking Methods, Done!\n";
    
    // Initiate the csvWriter
    std::ofstream csvWriter("output.csv");
    csvWriter << "simid, pid, effect, statistic, pvalue\n";
    
    int nSims = simConfig["--n-sims"];
    
    std::cout << std::endl;
    for (int i = 0; i < simConfig["--n-sims"]; i++) {
        
        while (journal.isStillAccepting()) {
            
            researcher.rest();
            
            researcher.experiment->allocateResources();
            researcher.experiment->generateData();
            researcher.experiment->calculateStatistics();
            researcher.experiment->calculateEffects();
            
            researcher.experiment->runTest();
            
            if (researcher.isHacker){
                researcher.hack();
            }
            
            researcher.prepareTheSubmission();
            
            researcher.submitToJournal();
            
            std::cout << researcher.submissionRecord << "\n";
            
            if (simConfig["--progress"])
                simulationBar.progress(i, nSims);

        }
        
        if (simConfig["--save-output"]){
            int pid = 0;
            for (auto& p : journal.submissionList) {
                p.simid = i;
                p.pubid = pid++;
                csvWriter << p << "\n";
            }
        }
        
        journal.clear();
    }
    
    if (simConfig["--save-output"]){
        csvWriter.close();
    }
    
    if (simConfig["--progress"])
        simulationBar.finish();
}

//void testDOCOPT(std::map<std::string, docopt::value> args){
//    json config;
//
//    for(auto const& arg : args) {
////        if (arg.second.isString())
//            config[arg.first] = arg.second;
////        std::cout << arg.first << ": " << arg.second << std::endl;
//    }
//
//    std::cout << config;
//
//}

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

//void effectEstimatorTest(){
//    StandardMeanDiffEffectEstimator _effect_estimator;
//    _effect_estimator.computeEffectFromStats(1, 2, 3, 4, 5, 6);
//}


void latentStrategyTest() {
    // Declaring sample paramters
    int nc = 1;
    int nd = 4;
    int nobs = 200;
    std::vector<double> means {.15, .15, 0.147, 0.147};
    std::vector<double> sds {0.01, 0.01, 0.01, 0.01};
//
//    int max_pubs = 70;
//    double alpha= 0.05;
//    double pub_bias = 0.95;
    
    
    RandomNumberGenerator rngEngine(42, false);
    
    ExperimentSetup estherSetup(nc, nd, nobs, means, sds);
    
    Experiment estherExperiment(estherSetup);
    
    LatentDataStrategy latentGen(rngEngine);
    estherExperiment.setDataStrategy(&latentGen);
    
//    latentGen.latentModelTest();
    
}


void estherSimulationTest(){

    // Declaring sample paramters
    int nc = 1;
    int nd = 4;
    int nobs = 200;
    std::vector<double> means {.15, .15, 0.147, 0.147};
    std::vector<double> sds {0.01, 0.01, 0.01, 0.01};

    int max_pubs = 70;
    double alpha= 0.05;
    double pub_bias = 0.95;


    RandomNumberGenerator rngEngine(42, false);

    ExperimentSetup estherSetup(nc, nd, nobs, means, sds);

    Experiment estherExperiment(estherSetup);

    FixedEffectStrategy fixedEffectModel(rngEngine);
    estherExperiment.setDataStrategy(&fixedEffectModel);

//    estherExperiment.initExperiment();
    estherExperiment.allocateResources();
    estherExperiment.generateData();
    estherExperiment.calculateStatistics();

    Researcher esther(&estherExperiment);

//    TTest tTest(&estherExperiment);
//    esther.setTestStrategy(&tTest);
//
//
//    // esther.calculateEffect();
//    esther.runTest();

    Journal journal(max_pubs, pub_bias, alpha);
    SignigicantSelection sigSelection(alpha, pub_bias);
    journal.setSelectionStrategy(&sigSelection);
//
    esther.setJournal(&journal);
//
    // esther.selectTheOutcome();
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

void estherSimulation(){
    int nc = 1;
    int nd = 4;
    int nobs = 25;
    int nsims = 1;
//    std::vector<double> means {.147, .147, 0.147, 0.147};
    std::vector<double> means {0.1, 0.3, 0.5, 0.7};
//    std::vector<double> means {.8, .9, 1., 2.};
    std::vector<double> sds {0.01, 0.01, 0.01, 0.01};

    int max_pubs = 20;
    double alpha= 0.05;
    double pub_bias = 0.95;

    RandomNumberGenerator rngEngine(42, false);

    ExperimentSetup estherSetup(nc, nd, nobs, means, sds);

    Experiment estherExperiment(estherSetup);

    FixedEffectStrategy fixedEffectModel(rngEngine);
    estherExperiment.setDataStrategy(&fixedEffectModel);

    Journal journal(max_pubs, pub_bias, alpha);
    SignigicantSelection sigSelection(alpha, pub_bias);
    journal.setSelectionStrategy(&sigSelection);

//    estherExperiment.initExperiment();
    Researcher esther(&estherExperiment);
//     esther.experiment->initExperiment();

//    OutcomeSwitching outSwitcher("min pvalue");
//    esther.isHacker = true;
//    esther.registerAHackingStrategy(&outSwitcher);

//    OptionalStopping optStopping(3000, 5);
//     esther.isHacker = false;
//    esther.registerAHackingStrategy(&optStopping);

    esther.setJournal(&journal);
    
    ReportPreregisteredGroup preRegReporter(0);
    esther.setDecisionStrategy(&preRegReporter);
    
//    Experiment copiedExpr = estherExperiment;
//    Experiment cpExpr = *esther.experiment;
//    esther.experiment->means[0] = 15;
//    std::cout << estherExperiment.means[0] << "\n";
////    cpExpr.means[0] = 10;
//    std::cout << cpExpr.means[0] << "\n";
    

    // I think this is a bit dangerous since I might lose track at some point,
    // It would be nicer if I can say `setTestStrategy(this)` or `setTestStrategy(parent)`
    TTest tTest;
    esther.experiment->setTestStrategy(&tTest);

    for (int i = 0; i < nsims; ++i) {

        while (journal.isStillAccepting()) {

            esther.rest();
//            simulationBar.progress(i * max_pubs + i, nsims * max_pubs);

            esther.submissionsList.clear();
//            esther.experiment->initExperiment();
            // These four are technically in initExperiment(),
            // I'll have them here individually for testing.
            esther.experiment->allocateResources();
            esther.experiment->generateData();
            esther.experiment->calculateStatistics();
            esther.experiment->calculateEffects();

            esther.experiment->runTest();
            
//            printVector(esther.experiment->means); std::cout << ": o, esther, means, [1]\n";
            if (esther.isHacker) {
                // std::cout << "hacking\n";
                esther.hack();
//                printVector(esther.experiment->means); std::cout << ": h, esther, means [3]\n";
//                printVector(esther.hackedExperiments[0].means); std::cout << ": hacked array, means [4]\n";
            }


            esther.prepareTheSubmission();
            
            esther.submitToJournal();

//           std::cout << esther.submissionRecord<< "\n";
        }

        journal.clear();
    }
//    simulationBar.finish();


}

void testRandomClass(){
    std::cout << std::endl;
    RandomNumberGenerator rngEngine(42, true);

    std::vector<double> means = {10, 20, 30};
    std::vector<std::vector<double >> sds = {{1, .5, 0}, {.5, 1, 0}, {0, 0, 1}};

    auto mvnormrng = rngEngine.mvnorm(means, sds);
    auto rng = rngEngine.mvnorm(means, sds, 100);

    for (auto &row : rng){
        std::cout << "mean: " << mean(row);
        std::cout << std::endl;
    }

    
    gsl_vector* mu = gsl_vector_alloc(3);
    mu->data = means.data();
    gsl_matrix* sigma = gsl_matrix_alloc(3, 3);
    gsl_matrix_set_all(sigma, 1);
    
    gsl_matrix* mvnorm_rans = gsl_matrix_alloc(3, 50);
    
    rngEngine.mvnorm_n(mu, sigma, mvnorm_rans);

}
