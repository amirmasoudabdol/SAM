//
// Created by Amir Masoud Abdol on 2019-04-20.
//

#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
//#include <filesystem>

#include "sam.h"

#include "effolkronium/random.hpp"
#include "spdlog/spdlog.h"
#include "utils/tqdm.h"
#include "nlohmann/json.hpp"

#include <indicators/block_progress_bar.hpp>
#include <indicators/progress_bar.hpp>
#include <indicators/termcolor.hpp>
#include <indicators/cursor_control.hpp>

#include "Researcher.h"
#include "PersistenceManager.h"

using namespace sam;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

using Random = effolkronium::random_static;

using namespace std;

void runSimulation(json &simConfig);

bool FLAGS::PROGRESS = false;
bool FLAGS::DEBUG = false;
bool FLAGS::UPDATECONFIG = false;

int main(int argc, const char **argv) {

  po::options_description desc("SAMrun Options");
  desc.add_options()("help", "produce help message")(
      "version", "SAMrun 0.0.1 (Alpha)")("verbose", po::bool_switch(),
                                         "Print more texts.")(
      "debug", po::bool_switch(), "Print debugging information")(
      "update-config", po::bool_switch()->default_value(true),
      "Update the config file with the drawn seeds")(
      "progress", po::bool_switch()->default_value(false),
      "Shows the progress bar")(
      "master-seed", po::value<int>()->default_value(42),
      "Set the master seed")("output-prefix", po::value<string>(),
                             "Output prefix used for saving files")(
      "output-path", po::value<string>(),
      "Output path")("config", po::value<string>(), "JSON config file");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  json configs;

  std::string configfilename;
  if (vm.count("config")) {
    configfilename = vm["config"].as<string>();
//    if (!exists(configfilename)){
//      throw std::invalid_argument("Config file doesn't exist");
//    }
  } else {
    configfilename = "/Users/amabdol/Projects/SAMpp/config_file.json";
  }

  std::ifstream configFile(configfilename);
  configFile >> configs;

  if (vm.count("output-path")) {
    configs["simulation_parameters"]["output_path"] =
        vm["output-path"].as<string>();
  } else {
    configs["simulation_parameters"]["output_path"] = "../outputs/";
  }

  if (vm.count("output-prefix")) {
    const string output_prefix = vm["output-prefix"].as<string>();
    configs["simulation_parameters"]["output_prefix"] = output_prefix;
  }
  
  if (vm.count("progress")) {
    FLAGS::PROGRESS = vm["progress"].as<bool>();
  }

  spdlog::set_pattern("[%R] %^[%l]%$ %v");

  auto log_level = static_cast<spdlog::level::level_enum>(configs["simulation_parameters"]["log_level"].get<LogLevel>());
  spdlog::set_level(log_level);
  
  // Overwriting the logging level if given in CLI
  if (vm.count("debug"))
    FLAGS::DEBUG = vm["debug"].as<bool>();
  
  
  /// Setting and saving the config file before starting the simulation
  int masterseed{0};
  if (configs["simulation_parameters"]["master_seed"] == "random") {
    masterseed = static_cast<int>(time(NULL));
    configs["simulation_parameters"]["master_seed"] = masterseed;
  } else {
    masterseed = configs["simulation_parameters"]["master_seed"].get<int>();
  }
  Random::seed(masterseed);
  
  /// Saving the updated config file if necessary
  if (vm.count("update-config")) {
    const bool update_config = vm["update-config"].as<bool>();
    if (update_config) {
      std::ofstream o(configfilename);
      o << std::setw(4) << configs << std::endl;
      o.close();
    }
  }

  runSimulation(configs);

  return 0;
}

void runSimulation(json &simConfig) {

  FLAGS::PROGRESS |= simConfig["simulation_parameters"]["progress"].get<bool>();
//  FLAGS::DEBUG |= simConfig["simulation_parameters"]["debug"].get<bool>();

  Researcher researcher =
      Researcher::create("Peter").fromConfigFile(simConfig).build();

  // Initiate the csvWriter
  // I need an interface for this
  bool is_saving_all_pubs = simConfig["simulation_parameters"]["save_all_pubs"];
  std::string pubsfilename =
      simConfig["simulation_parameters"]["output_path"].get<std::string>() +
      simConfig["simulation_parameters"]["output_prefix"].get<std::string>() +
      "_Publications.csv";

  bool is_saving_rejected = simConfig["simulation_parameters"]["save_rejected"];
  std::string rejectedfilename =
      simConfig["simulation_parameters"]["output_path"].get<std::string>() +
      simConfig["simulation_parameters"]["output_prefix"].get<std::string>() +
      "_Rejected.csv";

  bool is_saving_pubs_summaries_per_sim = simConfig["simulation_parameters"]["save_pubs_per_sim_summaries"];
  bool is_saving_summaries = simConfig["simulation_parameters"]["save_overall_summaries"];
  bool is_saving_meta = simConfig["simulation_parameters"]["save_meta"];

  int n_sims = simConfig["simulation_parameters"]["n_sims"];

  std::unique_ptr<PersistenceManager::Writer> pubswriter;
  std::unique_ptr<PersistenceManager::Writer> rejectedwriter;

  // Initializing the csv writer
  if (is_saving_all_pubs)
    pubswriter = std::make_unique<PersistenceManager::Writer>(pubsfilename);

  if (is_saving_rejected)
    rejectedwriter =
        std::make_unique<PersistenceManager::Writer>(rejectedfilename);

  indicators::show_console_cursor(false);
  
  indicators::BlockProgressBar sim_progress_bar{
      indicators::option::BarWidth{50},
      indicators::option::Start{"["},
      indicators::option::End{"]"},
      indicators::option::ForegroundColor{indicators::Color::yellow},
      indicators::option::ShowElapsedTime{true},
      indicators::option::ShowRemainingTime{true}};

  int n_pubs = simConfig["journal_parameters"]["max_pubs"];
  int n_total_pubs = n_sims;

  auto progress = 0.0f;
  // This loop can be parallelized
  spdlog::info("Starting the simulation...");
  for (int i = 0; i < n_sims; ++i) {

    spdlog::debug("---> Sim {}", i);

    float j{0};
    
    // Reseting the experiment Id, this is mainly for counting the number of trial
    // before collecting `k` publications...
    researcher.experiment->exprid = 0;
    
    // Doning research until Journal doesn't accept anything
    while (researcher.journal->isStillAccepting()) {

      spdlog::debug("---> Experiment #{}", j++);

      researcher.research();

      spdlog::debug("\n\n==========================================================================\n");
    }
    
    if (FLAGS::PROGRESS) {
      progress += 1. / n_sims;
      sim_progress_bar.set_progress(progress * 100);
    }

    if (is_saving_all_pubs) {
      pubswriter->write(researcher.journal->publications_list, i);
    }

    if (is_saving_rejected) {
      rejectedwriter->write(researcher.journal->rejection_list, i);
    }

    if (is_saving_summaries or is_saving_meta)
      researcher.journal->runMetaAnalysis();
    
    if (is_saving_meta)
      researcher.journal->saveMetaAnalysis();
    
    if (is_saving_pubs_summaries_per_sim) {
      researcher.journal->savePulicationsPerSimSummaries();
    }
    
    researcher.journal->clear();
  }
  
  if (is_saving_summaries) {
    researcher.journal->saveSummaries();
  }
  
  
  indicators::show_console_cursor(true);
  

}
