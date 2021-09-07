//===-- main.cpp - SAMrun Main Function -------------------------*- C++ -*-===//
//
// Part of SAM Project
// Created by Amir Masoud Abdol on 2019-04-20.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the main function of SAMrun executable. SAMrun is
/// a simple program that utilizes libsam, and runs a given config file.
///
//===----------------------------------------------------------------------===//

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>

#include <boost/filesystem.hpp>

#include "sam.h"

#include "effolkronium/random.hpp"
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include "rang/rang.hpp"

#include "indicators/indicators.hpp"

#include "PersistenceManager.h"
#include "Researcher.h"

using namespace sam;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

using Random = effolkronium::random_static;

using namespace std;

bool show_progress_bar{false};

void runSimulation(json &simConfig);

/// @brief SAMrun's main routine
///
/// This reads the config file as well as the command line parameters. If
/// necessary, it will overwrite some of the config parameters based on command
/// line values, and finally call runSimulation().
int main(int argc, const char **argv) {

  spdlog::set_pattern("[%R] %^[%l]%$ %v");

  po::options_description desc("SAMrun Options");
  desc.add_options()
    ("help,h", "produce help message")
    ("version,v", "print version string")
    (
      "log-level", po::value<std::string>(), "Level of logging.")
    (
      "update-config", po::bool_switch(),
      "Update the config file with the drawn seeds")
    (
      "progress", po::bool_switch()->default_value(false),
      "Shows the progress bar")
    (
      "master-seed", po::value<int>()->default_value(42),
      "Set the master seed")
    ("output-prefix", po::value<std::string>(),
                             "Output prefix used for saving files")
    (
      "output-path", po::value<std::string>(),
      "Output path")
    ("config", po::value<std::string>(), 
      "JSON config file");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  if (vm.count("version")) {
    std::cout << "SAMrun 0.1.0 (Beta)" << "\n";
    return 1;
  }

  json configs;

  std::string config_file_name{};
  if (vm.count("config")) {
    config_file_name = vm["config"].as<string>();
    if (!boost::filesystem::exists(config_file_name)){
      std::cerr << "SAMrun: " << rang::fg::red << rang::style::bold << "error: " << rang::style::reset << "config file does not exist." << std::endl;
      return(1);
    }
  }else{
    std::cerr << "SAMrun: " << rang::fg::red << rang::style::bold << "error: " << rang::style::reset << "no configuration file." << std::endl;
    return 1;
  }

  std::ifstream config_file(config_file_name);
  config_file >> configs;

  auto log_level = static_cast<spdlog::level::level_enum>(
      configs["simulation_parameters"]["log_level"].get<LogLevel>());
  spdlog::set_level(log_level);

  // Overwriting the logging level if given in CLI
  if (vm.count("log-level")) {
    const string debug = vm["log-level"].as<string>();
    if (debug == "trace")
      spdlog::set_level(spdlog::level::trace);
    else if (debug == "debug")
      spdlog::set_level(spdlog::level::debug);
    else if (debug == "info")
      spdlog::set_level(spdlog::level::info);
    else if (debug == "warn")
      spdlog::set_level(spdlog::level::warn);
    else if (debug == "err")
      spdlog::set_level(spdlog::level::err);
    else if (debug == "critical")
      spdlog::set_level(spdlog::level::critical);
    else
      spdlog::set_level(spdlog::level::off);
  }

  spdlog::info("Processing the configuration file...");

  std::string output_path{configs["simulation_parameters"]["output_path"]};
  if (vm.count("output-path")) {
    output_path = vm["output-path"].as<string>();
  }
  if (!boost::filesystem::exists(output_path)){
    spdlog::debug("Creating {} directory...", output_path);
    try {
      boost::filesystem::create_directory(output_path);
    } catch (boost::filesystem::filesystem_error &e) {
      std::cerr << "SAMrun: " << rang::fg::red << rang::style::bold << "error: " << rang::style::reset << "cannot create a directory in the given path.\n";
      std::cerr << e.what();
      exit(1);
    }
  }
  configs["simulation_parameters"]["output_path"] = output_path;

  if (vm.count("output-prefix")) {
    const string output_prefix = vm["output-prefix"].as<string>();
    configs["simulation_parameters"]["output_prefix"] = output_prefix;
  }

  if (vm.count("progress")) {
    show_progress_bar = vm["progress"].as<bool>();
  }
  
  // Seeding the RNG
  // ---------------

  // If the config file contains a unique seed number, that will be used to set the
  // master_seed, otherwise, it'll generate a random seed to be used.
  std::mt19937::result_type master_seed;
  if (configs["simulation_parameters"]["master_seed"] == "random"
      or !configs["simulation_parameters"].contains("master_seed")) {
    
    spdlog::debug("Generating the Master Seed");

    // Generating a random seed based on using random_device
    // based on https://stackoverflow.com/a/13446015/1141307
    std::random_device rd;
    master_seed =
        rd() ^
        ((std::mt19937::result_type)
             std::chrono::duration_cast<std::chrono::seconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count() +
         (std::mt19937::result_type)
             std::chrono::duration_cast<std::chrono::microseconds>(
                 std::chrono::high_resolution_clock::now().time_since_epoch())
                 .count());
    configs["simulation_parameters"]["master_seed"] = master_seed;
  } else {
    master_seed = configs["simulation_parameters"]["master_seed"].get<int>();
  }
  Random::seed(master_seed);
  arma::arma_rng::set_seed(master_seed);

  // Saving the updated config file, if necessary
  if (vm.count("update-config")) {
    const bool update_config = vm["update-config"].as<bool>();
    configs["simulation_parameters"]["update_config"] = update_config;
    if (update_config) {
      std::ofstream o(config_file_name);
      o << std::setw(4) << configs << std::endl;
      o.close();
    }
  }

  runSimulation(configs);

  return 0;
}


/// Configures an instance of the Researcher, initializes its internal and execute
/// the simulation. This is also responsible for initialization of the CSV files.
///
/// @param sim_configs simulation configurations
void runSimulation(json &sim_configs) {

  show_progress_bar |=
      sim_configs["simulation_parameters"]["progress"].get<bool>();

  spdlog::info("Initializing the Researcher...");
  Researcher researcher =
      Researcher::create("Sam").fromConfigFile(sim_configs).build();

  // Initiate the csvWriter
  // I need an interface for this
  bool is_saving_all_pubs = sim_configs["simulation_parameters"]["save_all_pubs"];
  std::string pubs_file_name =
      sim_configs["simulation_parameters"]["output_path"].get<std::string>() +
      sim_configs["simulation_parameters"]["output_prefix"].get<std::string>() +
      "_Publications.csv";

  bool is_saving_rejected = sim_configs["simulation_parameters"]["save_rejected"];
  std::string rejs_file_name =
      sim_configs["simulation_parameters"]["output_path"].get<std::string>() +
      sim_configs["simulation_parameters"]["output_prefix"].get<std::string>() +
      "_Rejected.csv";
  
  bool is_saving_every_experiment{false};
  std::string exprs_file_name;
  if (sim_configs["simulation_parameters"].contains("save_every_experiment")) {
    is_saving_every_experiment = sim_configs["simulation_parameters"]["save_every_experiment"];
    exprs_file_name =
      sim_configs["simulation_parameters"]["output_path"].get<std::string>() +
      sim_configs["simulation_parameters"]["output_prefix"].get<std::string>() +
      "_Experiments.csv";
  }
  

  bool is_saving_pubs_summaries_per_sim =
      sim_configs["simulation_parameters"]["save_pubs_per_sim_summaries"];
  bool is_saving_summaries =
      sim_configs["simulation_parameters"]["save_overall_summaries"];
  bool is_saving_meta = sim_configs["simulation_parameters"]["save_meta"];

  int n_sims = sim_configs["simulation_parameters"]["n_sims"];

  std::unique_ptr<PersistenceManager::Writer> pubs_writer;
  std::unique_ptr<PersistenceManager::Writer> rejs_writer;
  std::unique_ptr<PersistenceManager::Writer> experiment_writer;

  // Initializing the csv writers
  if (is_saving_all_pubs) {
    pubs_writer = std::make_unique<PersistenceManager::Writer>(pubs_file_name);
  }

  if (is_saving_rejected) {
    rejs_writer =
        std::make_unique<PersistenceManager::Writer>(rejs_file_name);
  }
  
  if (is_saving_every_experiment) {
    experiment_writer = std::make_unique<PersistenceManager::Writer>(exprs_file_name);
  }

  indicators::show_console_cursor(false);

  indicators::ProgressBar sim_progress_bar{
    indicators::option::BarWidth{50},
    indicators::option::Start{"["},
    indicators::option::End{"]"},
    indicators::option::ForegroundColor{indicators::Color::yellow},
    indicators::option::ShowElapsedTime{true},
    indicators::option::ShowRemainingTime{true},
    indicators::option::MaxProgress{n_sims}
  };

  
  // Main Simulation Loop
  // --------------------
  
  spdlog::info("Starting the simulation...");
  for (int i = 0; i < n_sims; ++i) {

    spdlog::trace("---> Sim {}", i);

    float j{0};

    // Resetting the experiment Id, this is mainly for counting the number of
    // trial before collecting `k` publications...
    researcher.experiment->exprid = 0;

    researcher.randomizeHackingStrategies();

    // Performing research until Journal doesn't accept anything
    while (researcher.journal->isStillAccepting()) {

      spdlog::trace("---> Experiment #{}", j++);

      researcher.research();

      researcher.experiment->exprid++;
      
      if (is_saving_every_experiment) {
        experiment_writer->write(researcher.experiment.get(), i);
      }

      spdlog::trace("\n\n======================================================"
                    "====================\n");
    }

    researcher.experiment->simid++;

    if (show_progress_bar) sim_progress_bar.tick();

    if (is_saving_all_pubs) {
      pubs_writer->write(researcher.journal->publications_list, i);
    }

    if (is_saving_rejected) {
      rejs_writer->write(researcher.journal->rejection_list, i);
    }

    if (is_saving_summaries or is_saving_meta)
      researcher.journal->runMetaAnalysis();

    if (is_saving_meta)
      researcher.journal->saveMetaAnalysis();

    if (is_saving_pubs_summaries_per_sim) {
      researcher.journal->savePublicationsPerSimSummaries();
    }

    researcher.journal->clear();
  }

  if (is_saving_summaries) {
    researcher.journal->saveSummaries();
  }

  indicators::show_console_cursor(true);
}
