#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <iomanip>

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

static const char USAGE[] =
R"(SAMpp

    Usage:
        SAMpp [--master-seed S] [--config FILE]
              [--n-conditions C] [--n-dep-vars D]
              [--n-items I]
              [--n-obs O] [--means M] [--var V]
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
        --var=V             List of variances for each group [default: 0.01]
        --cov-const=CV        Constant covariant [default: 0.5]
        --output-prefix=PREFIX    Output prefix used for saving files [default: auto]
        --output-path=PATH      Output path [default: output/]
        --config=FILE      JSON config file [default: /Users/amabdol/Projects/SAMpp/new_config_file.json]
        --is-p-hacker      If true, the Researcher will perform phacking techniques on the data [default: false]
        --hacking-methods-config=FILE  JSON config storing p-hacking methods and their parameters [default: ../sample_hacking_methods.json]
)";

// for convenience
void testRandomClass();
void latentStrategyTest();

void runSimulation(json& simConfig);

using json = nlohmann::json;

tqdm progressBar;

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

    bool verbose = simConfig["Simulation Parameters"]["--verbose"];
    bool progress = simConfig["Simulation Parameters"]["--progress"];
    bool debug = simConfig["Simulation Parameters"]["--debug"];
    bool output = simConfig["Simulation Parameters"]["--save-output"];

    if (simConfig["Simulation Parameters"]["--master-seed"] == 0) {
        srand(time(NULL));
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
    csvWriter << "simid, pid, tnobs, tyi, tvi, tcov, nobs, yi, sei, statistic, pvalue, side\n";
    
    int nSims = simConfig["Simulation Parameters"]["--n-sims"];
    
    std::cout << std::endl;
    for (int i = 0; i < simConfig["Simulation Parameters"]["--n-sims"]; i++) {
        
        while (researcher.journal->isStillAccepting()) {

            researcher.prepareResearch();
            
            researcher.performResearch();
            
            researcher.publishResearch();
            
            
            if (progress)
                progressBar.progress(i, nSims);

        }
        
        if (output){
            researcher.journal->saveSubmissions(i, csvWriter);
        }
        
        researcher.journal->clear();
    }

    if (progress)
        progressBar.finish();
    
    if (output){
        csvWriter.close();
        std::cout << "\nSaved to: " << outputfilename << "\n";
    }
    

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
