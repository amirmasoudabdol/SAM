#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <iomanip>

#include <main.h>

#include "Utilities.h"
#include "docopt.h"
#include "utils/tqdm.h"

#include <Experiment.h>
#include <ExperimentSetup.h>
#include <EffectEstimators.h>
#include <DataStrategy.h>
#include <TestStrategy.h>
#include <RandomNumberGenerator.h>
#include <Researcher.h>
#include <Journal.h>
#include <SelectionStrategies.h>
#include <HackingStrategy.h>
#include <DecisionStrategy.h>

#include "ioUtilities.h"

using json = nlohmann::json;

static const char USAGE[] =
R"(SAMpp

    Usage:
        SAMpp [--master-seed S] [--config FILE]
              [--output-path PATH] [--output-prefix PREFIX]
              [--progress] [--debug] [--verbose]

    Options:
        -h --help                   Show this screen.
        -v --version                Show version.
        --verbose                   Print more texts.
        --progress                  Show progress bar [default: false]
        --debug                     Print debugging information [default: false]
        --master-seed=S             Set the master seed [default: random]
        --output-prefix=PREFIX      Output prefix used for saving files [default: ]
        --output-path=PATH          Output path [default: ../outputs/]
        --config=FILE               JSON config file [default: /Users/amabdol/Projects/SAMpp/new_config_file.json]

)";

// for convenience
void testRandomClass();
void latentStrategyTest();

void runSimulation(json& simConfig);

using json = nlohmann::json;

tqdm progressBar;

extern bool VERBOSE;
extern bool PROGRESS;
extern bool DEBUG;
extern bool UPDATECONFIG;

int main(int argc, const char** argv){
    
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
                                                               { argv + 1, argv + argc },
                                                               true,                        // show help if requested
                                                               "SAMpp 0.1 (Alpha)");        // version string


    json jSimConfig = readJSON(args["--config"].asString());
    
    if (args.find("--output-path") != args.end()){
        jSimConfig["Simulation Parameters"]["--output-path"] = args["--output-path"].asString();
    }

    if (args["--output-prefix"].asString() != ""){
        jSimConfig["Simulation Parameters"]["--output-prefix"] = args["--output-prefix"].asString();
    }
    
    runSimulation(jSimConfig);
    
    if (UPDATECONFIG){
        std::ofstream o(args["--config"].asString());
        o << std::setw(4) << jSimConfig << std::endl;
    }

    return 0;
}

void runSimulation(json& simConfig){
    
    VERBOSE = simConfig["Simulation Parameters"]["--verbose"];
    PROGRESS = simConfig["Simulation Parameters"]["--progress"];
    DEBUG = simConfig["Simulation Parameters"]["--debug"];

    if (simConfig["Simulation Parameters"]["--master-seed"] == "random") {
        int masterseed = time(NULL);
        simConfig["Simulation Parameters"]["--master-seed"];
        srand(masterseed);
    }else{
        srand(simConfig["Simulation Parameters"]["--master-seed"]);
    }
    
    Researcher::Builder researcherBuilder;
    researcherBuilder.setConfig(simConfig)
                     .makeExperiment()
                     .makeJournal()
                     .makeDecisionStrategy()
                     .makeHackingStrategies();
    
    Researcher researcher = researcherBuilder.build();
    
    
    // Initiate the csvWriter
    // I need an interface for this
    std::string outputfilename = simConfig["Simulation Parameters"]["--output-path"].get<std::string>() + simConfig["Simulation Parameters"]["--output-prefix"].get<std::string>() + "_sim.csv";
    std::ofstream csvWriter( outputfilename );

    // Initializing the CSV header    
    csvWriter << Submission::header(simConfig["Experiment Parameters"]["--effect-estimators"]) << "\n";
    
    int nSims = simConfig["Simulation Parameters"]["--n-sims"];
    
    std::cout << std::endl;
    for (int i = 0; i < simConfig["Simulation Parameters"]["--n-sims"]; i++) {
        
        while (researcher.journal->isStillAccepting()) {

            researcher.prepareResearch();
            
            researcher.performResearch();
            
            researcher.publishResearch();
            
            if (VERBOSE) std::cout << std::endl;
            
            if (PROGRESS) progressBar.progress(i, nSims);

        }
        
        researcher.journal->saveSubmissions(i, csvWriter);
        
        researcher.journal->clear();
    }

    if (PROGRESS) progressBar.finish();
    
    csvWriter.close();
    std::cout << "\nSaved to: " << outputfilename << "\n";
    


}


//
//void testRandomClass(){
//    std::cout << std::endl;
//    RandomNumberGenerator rngEngine(42, true);
//
//    std::vector<double> means = {10, 20, 30};
//    std::vector<std::vector<double >> sds = {{1, .5, 0}, {.5, 1, 0}, {0, 0, 1}};
//
//    auto mvnormrng = rngEngine.mvnorm(means, sds);
//    auto rng = rngEngine.mvnorm(means, sds, 100);
//
//    for (auto &row : rng){
//        std::cout << "mean: " << mean(row);
//        std::cout << std::endl;
//    }
//
//
//    gsl_vector* mu = gsl_vector_alloc(3);
//    mu->data = means.data();
//    gsl_matrix* sigma = gsl_matrix_alloc(3, 3);
//    gsl_matrix_set_all(sigma, 1);
//
//    gsl_matrix* mvnorm_rans = gsl_matrix_alloc(3, 50);
//
//    rngEngine.mvnorm_n(mu, sigma, mvnorm_rans);
//
//}
