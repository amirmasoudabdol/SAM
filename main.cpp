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

    if (simConfig["Simulation Parameters"]["--master-seed"] == 0) {
        srand(time(NULL));
    }else{
        srand(simConfig["Simulation Parameters"]["--master-seed"]);
    }
    
    int nobsSeed = rand();
    simConfig["Simulation Parameters"]["--nobs-seed"] = nobsSeed;
    RandomNumberGenerator nobsGenerator(nobsSeed, false);
    
    Journal journal(simConfig["Journal Parameters"]);
    
    Experiment experiment(simConfig);

    // Initializing the Researcher
    Researcher researcher(&experiment);

        // Assigning the Journal
        researcher.setJournal(&journal);

        // Setting the Decision Strategy
        researcher.setDecisionStrategy(DecisionStrategy::buildDecisionStrategy(simConfig["Researcher Parameters"]["--decision-strategy"]));


    // Initializing Hacking Routines
    researcher.isHacker = simConfig["Researcher Parameters"]["--is-phacker"];

    
    // Registering Hacking Methods
    if (simConfig["Researcher Parameters"]["--is-phacker"]){
        
        // -------------------------
        // Using the Factory Pattern
        if (!simConfig["Researcher Parameters"]["--p-hacking-methods"].is_null()) {
            for (auto &item : simConfig["Researcher Parameters"]["--p-hacking-methods"]){

                
                if (item["type"] == "Outcome Switching") {
                    // std::cout << "Registering: " << item.dump(4) << "\n";
                    
                    // Set Researcher Selection Preference
                    // TODO: Generalize MEEEEE!
                    if (item["preference"] == "Min P-value"){
                        // std::cout << "MinPvalue\n";
                        researcher.selectionPref = ResearcherPreference::MinPvalue;
                        researcher.decisionStrategy->selectionPref = ResearcherPreference::MinPvalue;
                    } /* else if for other options */
                }else{
                    // std::cout << "Registering: " << item.dump(4) << "\n";
                    researcher.hackingStrategies.push_back(HackingStrategy::buildHackingMethod(item));
                }
            }
        }
        // if (verbose) std::cout << "Registering Hacking Methods, Done!\n"; 
    }
    
//    Researcher::Builder researcherBuilder;
//    researcherBuilder.setConfig(simConfig)
//                        .makeExperiment()
//                        .makeJournal()
//                        .makeDecisionStrategy()
//                        .makeHackingStrategies();
    
//    researcher = researcherBuilder.build();
    
    
    // Initiate the csvWriter
    std::string outputfilename = simConfig["Simulation Parameters"]["--output-path"].get<std::string>() + simConfig["Simulation Parameters"]["--output-prefix"].get<std::string>() + "_sim.csv";
    std::ofstream csvWriter( outputfilename );
    csvWriter << "simid, pid, tnobs, tyi, tsdi, tcov, nobs, yi, sei, statistic, pvalue, side\n";
    
    int nSims = simConfig["Simulation Parameters"]["--n-sims"];
    
    std::cout << std::endl;
    for (int i = 0; i < simConfig["Simulation Parameters"]["--n-sims"]; i++) {
        
        while (journal.isStillAccepting()) {
            
            // TODO: randomizeSetup might be better
            researcher.experiment->randomize();

            researcher.rest();
            
            researcher.experiment->allocateResources();
            researcher.experiment->generateData();
            researcher.experiment->calculateStatistics();
            researcher.experiment->calculateEffects();
            
            researcher.experiment->runTest();
            
            // TODO: This is the issue that I mentioned in [#50](https://github.com/amirmasoudabdol/SAMpp/issues/50)
            // The problem is that the researcher should have a function that basically run all these function before
            // start doing anything, even generateData ... should be in a function.
            // This is all being done because I wasn to leave the decision to the decsionStrategy and be able to
            // select one submission among many. If I don't do this, then I need to make a distinction between a
            // hacker making a decision and non-hacker makinga decision.
            Experiment e = *researcher.experiment;
            researcher.experimentsList.push_back(e);
            researcher.submissionsList.push_back(researcher.decisionStrategy->selectOutcome(e));
            
            if (researcher.isHacker){
                researcher.hack();
            }
            
            researcher.prepareTheSubmission();
            
            researcher.submitToJournal();
            

            
            if (progress)
                progressBar.progress(i, nSims);
            
            // Initializing a new experiment
            // TODO: The main loop needs some cleanup
            // FIXME: I don't work with the LatentModel! Something with the gsl_vector!

            

        }
        
        if (simConfig["Simulation Parameters"]["--save-output"]){
            int pid = 0;
            for (auto& p : journal.submissionList) {
                p.simid = i;
                p.pubid = pid++;
                
                if (simConfig["Simulation Parameters"]["--debug"])
                    std::cout << p << "\n";
                
                csvWriter << p << "\n";
            }
        }
        
        journal.clear();
    }

    if (progress)
        progressBar.finish();
    
    if (simConfig["Simulation Parameters"]["--save-output"]){
        csvWriter.close();
        std::cout << "\nSaved to: " << outputfilename << "\n";
    }
    

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
