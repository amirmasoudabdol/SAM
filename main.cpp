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

  spdlog::set_pattern("[%R] %^[%l]%$ %v");

  spdlog::set_level(spdlog::level::off);

  if (configs["simulation_parameters"]["verbose"].get<bool>())
    spdlog::set_level(spdlog::level::info);

  if (configs["simulation_parameters"]["debug"].get<bool>())
    spdlog::set_level(spdlog::level::debug);

  runSimulation(configs);

  if (vm.count("update-config")) {
    const bool update_config = vm["update-config"].as<bool>();
    if (update_config) {
      std::ofstream o(configfilename);
      o << std::setw(4) << configs << std::endl;
    }
  }

  return 0;
}

void runSimulation(json &simConfig) {

  FLAGS::PROGRESS = simConfig["simulation_parameters"]["progress"];
  FLAGS::DEBUG = simConfig["simulation_parameters"]["debug"];

  int masterseed{0};
  if (simConfig["simulation_parameters"]["master_seed"] == "random") {
    masterseed = static_cast<int>(time(NULL));
    simConfig["simulation_parameters"]["master_seed"] = masterseed;
  } else {
    masterseed = simConfig["simulation_parameters"]["master_seed"].get<int>();
  }
  Random::seed(masterseed);

  Researcher researcher =
      Researcher::create("Peter").fromConfigFile(simConfig).build();

  // Initiate the csvWriter
  // I need an interface for this
  bool is_saving_pubs = simConfig["simulation_parameters"]["save_pubs"];
  std::string pubsfilename =
      simConfig["simulation_parameters"]["output_path"].get<std::string>() +
      simConfig["simulation_parameters"]["output_prefix"].get<std::string>() +
      "_pubs.csv";

  bool is_saving_rejected = simConfig["simulation_parameters"]["save_rejected"];
  std::string rejectedfilename =
      simConfig["simulation_parameters"]["output_path"].get<std::string>() +
      simConfig["simulation_parameters"]["output_prefix"].get<std::string>() +
      "_rejected.csv";

  bool is_saving_stats = simConfig["simulation_parameters"]["save_stats"];
  std::string statsfilename =
      simConfig["simulation_parameters"]["output_path"].get<std::string>() +
      simConfig["simulation_parameters"]["output_prefix"].get<std::string>() +
      "_stats.csv";

  bool is_saving_sims = simConfig["simulation_parameters"]["save_sims"];
  std::string simsfilename =
      simConfig["simulation_parameters"]["output_path"].get<std::string>() +
      simConfig["simulation_parameters"]["output_prefix"].get<std::string>() +
      "_sims.csv";

  int n_sims = simConfig["simulation_parameters"]["n_sims"];

  std::unique_ptr<PersistenceManager::Writer> pubswriter;
  std::unique_ptr<PersistenceManager::Writer> rejectedwriter;
  std::unique_ptr<PersistenceManager::Writer> statswriter;
  std::unique_ptr<PersistenceManager::Writer> simswriter;

  // Initializing the csv writer
  if (is_saving_pubs)
    pubswriter = std::make_unique<PersistenceManager::Writer>(pubsfilename);

  if (is_saving_rejected)
    rejectedwriter =
        std::make_unique<PersistenceManager::Writer>(rejectedfilename);

  if (is_saving_stats)
    statswriter = std::make_unique<PersistenceManager::Writer>(statsfilename);

  if (is_saving_sims)
    simswriter = std::make_unique<PersistenceManager::Writer>(simsfilename);

  // Hide cursor
  std::cout << "\e[?25l";
  indicators::BlockProgressBar sim_progress_bar{
      indicators::option::BarWidth{50},
      indicators::option::Start{"["},
      indicators::option::End{"]"},
      indicators::option::ForegroundColor{indicators::Color::yellow},
      indicators::option::ShowElapsedTime{true},
      indicators::option::ShowRemainingTime{true}};

  int n_pubs = simConfig["journal_parameters"]["max_pubs"];
  int n_total_pubs = n_sims * n_pubs;

  auto progress = 0.0f;
  // This loop can be parallelized
  for (int i = 0; i < n_sims; ++i) {

    spdlog::debug("---> Sim {}", i);

    float j{0};
    while (researcher.journal->isStillAccepting()) {

      spdlog::debug("---> Experiment #{}", j++);

      researcher.research();

      // If Experiment handles the Submission, it can handle the
      // stats output as well.
      if (is_saving_stats)
        statswriter->write(researcher.experiment, "stats", i);

      if (FLAGS::PROGRESS) {
        progress += 1. / n_total_pubs;
        sim_progress_bar.set_progress(progress * 100);
      }

      spdlog::debug("\n\n==========================================================================\n");
    }

    if (is_saving_pubs) {
      pubswriter->write(researcher.journal->publications_list, i);
    }

    if (is_saving_rejected) {
      rejectedwriter->write(researcher.journal->rejection_list, i);
    }

    researcher.journal->clear();
  }

  // Show cursor
  std::cout << "\e[?25h";

  //  if (FLAGS::PROGRESS)
  //    progress_bar.finish();

  // if (is_saving_pubs)
  //     std::cout << "\nSaved to: " << pubsfilename << "\n";

  // if (is_saving_rejected)
  //     std::cout << "\nSaved to: " << rejectedfilename << "\n";

  // if (is_saving_stats)
  //     std::cout << "\nSaved to: " << statsfilename << "\n";

  // if (is_saving_sims)
  //     std::cout << "\nSaved to: " << simsfilename << "\n";
}
