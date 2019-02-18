#include <iostream>
#include <fstream>
#include <sstream>
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
        --config=FILE      JSON config file [default: /Users/amabdol/Projects/SAMpp/new_config_file.json]
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

    runSimulation(jSimConfig);

    return 0;
}

void runSimulation(json& simConfig){
    
    std::stringstream output_path_file;
    
    output_path_file << simConfig["--output-path"] << "_";

    if (simConfig["--master-seed"] == 0) {
        srand(time(NULL));
    }else{
        srand(simConfig["--master-seed"]);
    }
    
    int nobsSeed = rand();
    simConfig["--nobs-seed"] = nobsSeed;
    RandomNumberGenerator nobsGenerator(nobsSeed, false);


    ExperimentSetup experimentSetup(simConfig["Experiment Parameters"]);
    
    Journal journal(simConfig["Journal Parameters"]);
    
        // Setting Journal's Selection Strategy
    // TODO: Perhaps Journal should create this itself!
        journal.setSelectionStrategy(SelectionStrategy::buildSelectionStrategy(simConfig["Journal Parameters"]));
        if (simConfig["--verbose"]) std::cout << "Initializing Journal, Done!\n";

    // Initializing Experiment
    Experiment experiment(experimentSetup);
    if (simConfig["--verbose"]) std::cout << "Initializing Experiment, Done!\n";
    
        experiment.setDataStrategy(DataGenStrategy::buildDataStrategy(experimentSetup));
    
        if (simConfig["--verbose"]) std::cout << "Setting Data Model, Done!\n";
    
    
        // Setting the Test Strategy
        TTest tTest;
        experiment.setTestStrategy(&tTest);
        if (simConfig["--verbose"]) std::cout << "Setting the Test Strategy, Done!\n";

    // Initializing the Researcher
    Researcher researcher(&experiment);
    if (simConfig["--verbose"]) std::cout << "Initializing the Researcher, Done!\n";

        // Setting the Selection Preference
        researcher.selectionPref = PreRegisteredOutcome;
        if (simConfig["--verbose"]) std::cout << "Setting the Selection Preference, Done!\n";

        // Assigning the Journal
        researcher.setJournal(&journal);
        if (simConfig["--verbose"]) std::cout << "Assigning the Journal, Done!\n";

        // Setting the Decision Strategy
        ImpatientDecisionMaker impatientReporter(0, simConfig["Journal Parameters"]["--alpha"], researcher.selectionPref);
        researcher.setDecisionStrategy(&impatientReporter);
        if (simConfig["--verbose"]) std::cout << "Setting the Decision Strategy, Done!\n";

    // Initializing Hacking Routines
    researcher.isHacker = simConfig["Researcher Parameters"]["--is-phacker"];

    if (simConfig["--verbose"]) std::cout << "Initializing Hacking Routines, Done!\n";
    
    output_path_file << simConfig["Researcher Parameters"]["--is-phacker"] << "_";

    // Registering Hacking Methods
    if (simConfig["Researcher Parameters"]["--is-phacker"]){
        
        // Overwriting the selection preference, this is technically works as the
        // researcher is performing _Outcome Switching_.
        if (simConfig["Researcher Parameters"]["--selection-pref"] == "MinPvalue"){
            researcher.selectionPref = MinPvalue;
            researcher.decisionStrategy->selectionPref = MinPvalue;
        } /* else if for other options */
        
        
        // -------------------------
        // Using the Factory Pattern
        if (!simConfig["Researcher Parameters"]["--p-hacking-methods"].is_null()) {
            for (auto &item : simConfig["Researcher Parameters"]["--p-hacking-methods"]){
                researcher.hackingStrategies.push_back(HackingStrategy::buildHackingMethod(item));
                
                output_path_file << item["type"] << "_";
            }
        }
        if (simConfig["--verbose"]) std::cout << "Registering Hacking Methods, Done!\n";
    }
    
    
    // Initiate the csvWriter
    std::string outputfilename = simConfig["--output-path"].get<std::string>() + simConfig["--output-prefix"].get<std::string>() + ".csv";
    std::ofstream csvWriter( outputfilename );
    csvWriter << "simid, pid, nobs, yi, sei, statistic, pvalue, side\n";
    
    int nSims = simConfig["--n-sims"];
    
    std::cout << std::endl;
    for (int i = 0; i < simConfig["--n-sims"]; i++) {
        
        while (journal.isStillAccepting()) {
            
            if (simConfig["Experiment Parameters"]["--n-obs"] == 0){
                researcher.experiment->setup.nobs = nobsGenerator.genSampleSize(.75, 20, 100, 300);
            }
            
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
            

            
            if (simConfig["--progress"])
                simulationBar.progress(i, nSims);
            
            // Initializing a new experiment
            // TODO: The main loop needs some cleanup
            // FIXME: I don't work with the LatentModel! Something with the gsl_vector!

            
//            researcher.experiment->setup.setNObs(nobsGenerator.genSampleSize(.75, 20, 100, 300));

        }
        
        if (simConfig["--save-output"]){
            int pid = 0;
            for (auto& p : journal.submissionList) {
                p.simid = i;
                p.pubid = pid++;
                
                if (simConfig["--debug"])
                    std::cout << p << "\n";
                
                csvWriter << p << "\n";
            }
        }
        
        journal.clear();
    }

    if (simConfig["--progress"])
        simulationBar.finish();
    
    if (simConfig["--save-output"]){
        csvWriter.close();
        std::cout << "\nSaved to: " << outputfilename << "\n";
    }
    

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

void testTTest(){
    auto ttest_res = oneSampleTTest(15, sqrt(87.5), 13, 12, sqrt(39), 11, true);
    std::cout << "t: " << ttest_res.first << "p: " << ttest_res.second << std::endl;
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
