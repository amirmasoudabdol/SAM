//
// Created by Amir Masoud Abdol on 2019-04-20.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <iomanip>

#include "sam.h"

#include "utils/tqdm.h"
#include "effolkronium/random.hpp"

#include "Researcher.h"
#include "MetaAnalysis.h"
#include "PersistenceManager.h"

using namespace sam;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

using Random = effolkronium::random_static;


using namespace std;

void runSimulation(json& simConfig);

tqdm progressBar;

bool FLAGS::VERBOSE = false;
bool FLAGS::PROGRESS = false;
bool FLAGS::DEBUG = false;
bool FLAGS::UPDATECONFIG = false;

int main(int argc, const char** argv){
    
    po::options_description desc("SAMrun Options");
    desc.add_options()
        ("help",
            "produce help message")
        ("version",
            "SAMrun 0.0.1 (Alpha)")
        ("verbose",
            po::bool_switch(),
            "Print more texts.")
        ("debug",
            po::bool_switch(),
            "Print debugging information")
        ("update-config",
            po::bool_switch()->default_value(true),
            "Update the config file with the drawn seeds")
        ("master-seed",
            po::value<int>()->default_value(42),
            "Set the master seed")
        ("output-prefix",
            po::value<string>(),
            "Output prefix used for saving files")
        ("output-path",
            po::value<string>(),
            "Output path")
        ("config",
            po::value<string>(),
            "JSON config file")
    ;
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    json jSimConfig;
    
    string configfilename;
    if (vm.count("config")){
        configfilename = vm["config"].as<string>();
    }else{
        configfilename = "/Users/amabdol/Projects/SAMpp/config_file.json";
    }
    
    std::ifstream configFile(configfilename);
    configFile >> jSimConfig;

    if (vm.count("output-path")){
        jSimConfig["SimulationParameters"]["output_path"] = vm["output-path"].as<string>();
    }else{
        jSimConfig["SimulationParameters"]["output_path"] = "../outputs/";
    }

    if (vm.count("output-prefix")){
        const string output_prefix = vm["output-prefix"].as<string>();
        jSimConfig["SimulationParameters"]["output_prefix"] = output_prefix;
    }else{
        jSimConfig["SimulationParameters"]["output_prefix"] = "";
    }
    
    runSimulation(jSimConfig);

    if (vm.count("update-config")){
        const bool update_config = vm["update-config"].as<bool>();
        if (update_config){
            std::ofstream o(configfilename);
            o << std::setw(4) << jSimConfig << std::endl;

        }
    }

    return 0;
}

void runSimulation(json& simConfig){

    FLAGS::VERBOSE = simConfig["SimulationParameters"]["verbose"];
    FLAGS::PROGRESS = simConfig["SimulationParameters"]["progress"];
    FLAGS::DEBUG = simConfig["SimulationParameters"]["debug"];

    int masterseed {0};
    if (simConfig["SimulationParameters"]["master_seed"] == "random") {
        simConfig["SimulationParameters"]["master_seed"] = masterseed;
        masterseed = static_cast<int>(time(NULL));
        Random::seed(masterseed);
    }else{
        masterseed = simConfig["SimulationParameters"]["master_seed"];
        Random::seed(masterseed);
    }
    
    Researcher researcher = Researcher::create("Peter")
                                        .fromConfigFile(simConfig)
                                        .build();

    // Initiate the csvWriter
    // I need an interface for this
    bool is_saving_pubs = simConfig["SimulationParameters"]["save_pubs"];
    std::string pubsfilename = simConfig["SimulationParameters"]["output_path"].get<std::string>() +\
                                 simConfig["SimulationParameters"]["output_prefix"].get<std::string>() +\
                                 "_pubs.csv";
    
    bool is_saving_rejected = simConfig["SimulationParameters"]["save_rejected"];
    std::string rejectedfilename = simConfig["SimulationParameters"]["output_path"].get<std::string>() +\
                                 simConfig["SimulationParameters"]["output_prefix"].get<std::string>() +\
                                 "_rejected.csv";
    
    bool is_saving_stats = simConfig["SimulationParameters"]["save_stats"];
    std::string statsfilename = simConfig["SimulationParameters"]["output_path"].get<std::string>() +\
                                simConfig["SimulationParameters"]["output_prefix"].get<std::string>() +\
                                "_stats.csv";
    
    bool is_saving_sims = simConfig["SimulationParameters"]["save_sims"];
    std::string simsfilename = simConfig["SimulationParameters"]["output_path"].get<std::string>() +\
                                simConfig["SimulationParameters"]["output_prefix"].get<std::string>() +\
                                "_sims.csv";

    int nSims = simConfig["SimulationParameters"]["n_sims"];
    
    std::unique_ptr<PersistenceManager::Writer> pubswriter;
    std::unique_ptr<PersistenceManager::Writer> rejectedwriter;
    std::unique_ptr<PersistenceManager::Writer> statswriter;
    std::unique_ptr<PersistenceManager::Writer> simswriter;
    
    // Initializing the csv writer
    if (is_saving_pubs)
        pubswriter = std::make_unique<PersistenceManager::Writer>(pubsfilename);
    
    if (is_saving_pubs)
        rejectedwriter = std::make_unique<PersistenceManager::Writer>(rejectedfilename);
    
    if (is_saving_stats)
        statswriter = std::make_unique<PersistenceManager::Writer>(statsfilename);
    
    if (is_saving_sims)
        simswriter = std::make_unique<PersistenceManager::Writer>(simsfilename);
    
//    std::cout << std::endl;
    for (int i = 0; i < simConfig["SimulationParameters"]["n_sims"]; i++) {

        while (researcher.journal->isStillAccepting()) {

            researcher.prepareResearch();

            researcher.performResearch();

            researcher.publishResearch();
            
            // If Experiment handles the Submission, it can handle the
            // stats output as well.
            // TODO: `i` should be adjusted
            if (is_saving_stats)
                statswriter->write(researcher.experiment, "stats", i);

            if (FLAGS::PROGRESS)
                progressBar.progress(i, nSims);
            
        }
        
        if (is_saving_pubs){
            pubswriter->write(researcher.journal->publications_list, i);
        }
        
        if (is_saving_rejected){
            rejectedwriter->write(researcher.journal->rejection_list, i);
        }
        
        researcher.journal->clear();
    }

    if (FLAGS::PROGRESS) progressBar.finish();

    if (FLAGS::VERBOSE){
        if (is_saving_pubs)
            std::cout << "\nSaved to: " << pubsfilename << "\n";
        
        if (is_saving_rejected)
            std::cout << "\nSaved to: " << rejectedfilename << "\n";
        
        if (is_saving_stats)
            std::cout << "\nSaved to: " << statsfilename << "\n";
        
        if (is_saving_sims)
            std::cout << "\nSaved to: " << simsfilename << "\n";
    }
}
