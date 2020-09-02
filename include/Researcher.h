//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_RESEARCHER_H
#define SAMPP_RESEARCHER_H

#include "DecisionStrategy.h"
#include "Experiment.h"
#include "HackingStrategy.h"
#include "Journal.h"
#include "Submission.h"
#include "HackingProbabilityStrategy.h"

using Random = effolkronium::random_static;

namespace sam {

class ResearcherBuilder;

using HackingWorkflow = std::vector<std::vector<std::variant<std::shared_ptr<HackingStrategy>, PolicyChain, PolicyChainSet>>>;

class Researcher {
  // Making the ResearcherBuilder a friend class in order to give it access to
  // private memebers of the Researcher. At the moment it's only `name` but I'll
  // encapsulate more variables as the time goes.
  friend class ResearcherBuilder;

  //! An arbitrary name for the Researcher
  std::string name;

public:
  ///
  /// Default constructor of the Researcher.
  ///
  /// \note       This is defined `private` because I want force the user to use
  /// the
  ///             `create(name)` method and therefore delegate the construction
  ///             to the ResearcherBuilder.
  ///
  Researcher() = default;

  ///
  /// Starts the Researcher build process. Use this to build a new instance of
  /// the Researcher.
  ///
  /// \param      name  The researcher name
  /// \return     An instance of ResearcherBuilder.
  ///
  static ResearcherBuilder create(std::string name);

  std::unique_ptr<Experiment> experiment;
  std::shared_ptr<Journal> journal;
  std::unique_ptr<DecisionStrategy> decision_strategy;
  std::vector<std::vector<std::unique_ptr<HackingStrategy>>> hacking_strategies;

  bool is_pre_processing{false};
  std::vector<std::unique_ptr<HackingStrategy>> pre_processing_methods;

  bool isHacker();
  
  bool isCommittingToTheHack(HackingStrategy *hs);
  
  
  //! Number of hacking strategies to be choosen from the given list of strategies
  int n_hacks;
  
  //! Indicates order in which hacking strategies are going to be selected
  //! from the list of given hacking strategies if the Researcher decides has
  //! to apply a fewer number than the given list
  std::string hacking_selection_priority {"order"};
  
  //! Indicates the execution order of selected/given hacking strategies
  std::string hacking_execution_order {"sequential"};
  
  //! Indicates the probablity of a Researcher _deciding_ to go for hacking an Experiment
  std::variant<double, Distribution> probability_of_being_a_hacker;
  
  //! Indicates the probablity of a Researcher _actally applying_ a choosen hacking strategy
  std::variant<double, std::string, Distribution, std::unique_ptr<HackingProbabilityStrategy>> probability_of_committing_a_hack;
  
  Parameter<double> hacking_probability;
  
  double submission_probability {1};

  //! A Submission record that Researcher is going to submit to the Journal
  SubmissionPool submissions_from_reps;

  void preProcessData();
  
  HackingWorkflow h_workflow;
  
  void research();
  void letTheHackBegin();
  
  
  void checkAndsubmitTheResearch(const std::optional<Submission> &sub);

  // This could be renamed to something like, selectThePreferedSubmission()
//  void prepareTheSubmission();
//  void submitToJournal();
  
  void computeStuff() {
    experiment->calculateStatistics();
    experiment->calculateEffects();
    experiment->runTest();
    
    for (int g{0}; g < experiment->setup.ng(); ++g)
      spdlog::debug("{}: ", (*experiment)[g]);
  }

  ///
  /// Set the decisionStrategy of the researcher.
  ///
  /// \param      d     The pointer to a Decision Strategy
  ///
  /// \note       Researcher owns its decision strategy that's why I move the
  ///             pointer.
  ///
  /// \todo       I think I need to do the `std::move` when I'm calling the
  /// function
  ///             not inside it
  ///
  void setDecisionStrategy(std::unique_ptr<DecisionStrategy> ds) {
    decision_strategy = std::move(ds);
  }

  ///
  /// Set the experiment. This can be used to setup several researchers with one
  /// experiment.
  ///
  /// \param      e     The pointer to an Experiment
  ///
//  void setExperiment(Experiment *e) { experiment = e; };

  ///
  /// \brief      Set the Jouranl
  ///
  /// \param      j     The pointer to a Journal instance
  ///
//  void setJournal(Journal *j) { journal = j; };
};

///
/// ResearcherBuilder class for Researcher. This takes care of eveyrthing and
/// return a fully initialized Researcher after calling `.build()` method.
///
class ResearcherBuilder {

  Researcher researcher;

  json config;
  bool build_from_config = false;

public:
  ResearcherBuilder(){};

  ResearcherBuilder(std::string name) { researcher.name = name; };

  ///
  /// Build a researcher entirely based on the given config file. This is not
  /// the best implementation still but I think it's more readable and
  /// reasonable for some usecases.
  ///
  /// \param      config  A JSON object
  /// \return     Return an instance of itself
  ///
  ResearcherBuilder &fromConfigFile(json &config) {

    this->config = config;

    /// \todo I can technically replace all these direct calls with calls
    /// to their counterpart in the Builder!

    researcher.experiment = std::make_unique<Experiment>(config["experiment_parameters"]);

    config["journal_parameters"]["save_meta"] = config["simulation_parameters"]["save_meta"];
    config["journal_parameters"]["save_pubs_per_sim_summaries"] = config["simulation_parameters"]["save_pubs_per_sim_summaries"];
    config["journal_parameters"]["save_overall_summaries"] = config["simulation_parameters"]["save_overall_summaries"];
    
    config["journal_parameters"]["output_path"] = config["simulation_parameters"]["output_path"];
    config["journal_parameters"]["output_prefix"] = config["simulation_parameters"]["output_prefix"];
    researcher.journal = std::make_shared<Journal>(config["journal_parameters"]);

    researcher.decision_strategy = DecisionStrategy::build(
        config["researcher_parameters"]["decision_strategy"]);

    researcher.is_pre_processing =
        config["researcher_parameters"]["is_pre_processing"];
    if (researcher.is_pre_processing) {
      for (auto &item :
           config["researcher_parameters"]["pre_processing_methods"]) {

        researcher.pre_processing_methods.push_back(
            HackingStrategy::build(item));
      }
    }    
    
    auto prob_of_being_a_hacker = config["researcher_parameters"]["probability_of_being_a_hacker"];
    switch (prob_of_being_a_hacker.type()) {
      case nlohmann::detail::value_t::number_integer:
      case nlohmann::detail::value_t::number_unsigned:
      case nlohmann::detail::value_t::number_float:
        researcher.probability_of_being_a_hacker = prob_of_being_a_hacker.get<double>();
        break;
      case nlohmann::detail::value_t::object:
        researcher.probability_of_being_a_hacker = make_distribution(prob_of_being_a_hacker);
        break;
      default:
        researcher.probability_of_being_a_hacker = 0.;
        break;
    }
    
    
    auto prob_of_committing_a_hack = config["researcher_parameters"]["probability_of_committing_a_hack"];
    switch (prob_of_committing_a_hack.type()) {
      case nlohmann::detail::value_t::number_integer:
      case nlohmann::detail::value_t::number_unsigned:
      case nlohmann::detail::value_t::number_float:
        researcher.probability_of_committing_a_hack = prob_of_committing_a_hack.get<double>();
        break;
      case nlohmann::detail::value_t::string:
        researcher.probability_of_committing_a_hack = prob_of_committing_a_hack.get<std::string>();
        break;
      case nlohmann::detail::value_t::object: {
        if (prob_of_committing_a_hack.contains("dist"))
          researcher.probability_of_committing_a_hack = make_distribution(prob_of_committing_a_hack);
        else
          researcher.probability_of_committing_a_hack = HackingProbabilityStrategy::build(prob_of_committing_a_hack);
      }
        break;
      default:
        researcher.probability_of_committing_a_hack = 0.;
        break;
    }
    
    // Parsing Hacking Strategies      
      researcher.h_workflow.resize(config["researcher_parameters"]["hacking_strategies"].size());
        
        for (int h {0}; h < researcher.h_workflow.size(); ++h ) {
                  
          auto &item = config["researcher_parameters"]["hacking_strategies"][h];
          
          researcher.h_workflow[h].push_back(HackingStrategy::build(item[0]));

          researcher.h_workflow[h].push_back(PolicyChainSet{item[1].get<std::vector<std::vector<std::string>>>(), researcher.decision_strategy->lua});

          researcher.h_workflow[h].push_back(PolicyChain{item[2].get<std::vector<std::string>>(), researcher.decision_strategy->lua});
      }

    
    if (config["researcher_parameters"].contains("n_hacks")) {
      researcher.n_hacks = config["researcher_parameters"]["n_hacks"].get<int>();
      
      // clamping the value if it's greater than the number of methods given
//      researcher.n_hacks = std::max(researcher.h_workflow.size(), researcher.n_hacks);
      
    } else {
      researcher.n_hacks = static_cast<int>(researcher.h_workflow.size());
    }
    
    
    //! Selecting between hacking
    if (config["researcher_parameters"].contains("hacking_selection_priority")) {
      researcher.hacking_selection_priority = config["researcher_parameters"]["hacking_selection_priority"].get<std::string>();
      /// \todo Handle the selection based on n_hacks
    }
    
    
    //! Setting the execution order
    if (config["researcher_parameters"].contains("hacking_execution_order")) {
      researcher.hacking_execution_order = config["researcher_parameters"]["hacking_execution_order"].get<std::string>();
      /// \todo Handle the ordering
    }
    
    
    /// \note This is not yet implemented, so, I made it optional for now. I'm thinking to
    /// implement this together with hacking probability strategy as a variant
    /// std::variant<Parameter<double>, std::unique_ptr<HackingProbabilityStrategy>>
    if (config["researcher_parameters"].contains("hacking_probability"))
      researcher.hacking_probability = Parameter<double>(config["researcher_parameters"]["hacking_probability"],
                                                       1);
    
    /// Indicate what percentage of Researcher's work is going to be submitted
    if (config["researcher_parameters"].contains("submission_probability"))
      researcher.submission_probability = config["researcher_parameters"]["submission_probability"].get<double>();
    
    build_from_config = true;

    return *this;
  }

  ////////////////////////////
  /// CREATEING NEW OBJECT ///
  ////////////////////////////

  ///
  /// \brief      Create a new DecisionStrategy for the researcher based on the
  /// given configuration.
  ///
  ResearcherBuilder &createDecisionStrategy(json &ds) {
    researcher.decision_strategy = DecisionStrategy::build(ds);
    return *this;
  }

  ///
  /// \brief      Create a new Journal for the researcher based on the given
  ///             configuration.
  ///
  /// \note       The configuration needs to include information about the
  ///             SelectionStrategy as well.
  ///
//  ResearcherBuilder &createJournal(json &journal_config) {
//    researcher.journal = new Journal(journal_config);
//    return *this;
//  }

  ///
  /// \brief      Create a new Experiment based on the given ExperimentSetup.
  ///
  /// \note       : This assumes that the experiment setup is correctly
  /// initiated.
  ///
//  ResearcherBuilder &createExperiment(ExperimentSetup es) {
//    researcher.experiment = new Experiment(es);
//    return *this;
//  }

  ResearcherBuilder &
  createNewHackingStrategyGroup(json &hacking_strategy_group_config) {

    return *this;
  }

  ResearcherBuilder &createNewHackingStrategy(json &hacking_strategy_config) {

    return *this;
  }

  //////////////////////////
  /// SETTING NEW OBJECT ///
  //////////////////////////

  //        ResearcherBuilder& setDecisionStrategy(const DecisionStrategy &ds){
  //            researcher.decision_strategy = ds;
  //            return *this;
  //        };

//  ResearcherBuilder &setExperiment(Experiment *expr) {
//    researcher.experiment = expr;
//    return *this;
//  }
//
//  ResearcherBuilder &setExperimentSetup(ExperimentSetup es) {
//    researcher.experiment = new Experiment(es);
//    return *this;
//  };

  // I'm not sure if I should leave these here or not
  ResearcherBuilder &setDataStrategy(std::shared_ptr<DataStrategy> ds) {
    researcher.experiment->data_strategy = ds;
    return *this;
  }

  // I'm not sure if I should leave these here or not
  ResearcherBuilder &setTestStrategy(std::shared_ptr<TestStrategy> &ts) {
    researcher.experiment->test_strategy = ts;
    return *this;
  };

//  ResearcherBuilder &setJournal(Journal *j) {
//    researcher.journal = j;
//    return *this;
//  };

  ResearcherBuilder &
  setJournalSelectionStrategy(std::unique_ptr<SelectionStrategy> ss) {

    researcher.journal->setSelectionStrategy(std::move(ss));
    return *this;
  };

  ResearcherBuilder &setDecisionStrategy(std::unique_ptr<DecisionStrategy> ds) {
    researcher.setDecisionStrategy(std::move(ds));
    return *this;
  };

  ResearcherBuilder &setHackingStrategy(HackingStrategy *hs);
  ResearcherBuilder &
      setHackingStrategy(std::vector<std::vector<HackingStrategy *>>);

  ResearcherBuilder &
  addHackingStrategyGroup(std::vector<HackingStrategy *> hsg) {
    //            researcher.hacking_strategies.push_back(hsg);
    return *this;
  }

  ResearcherBuilder &addNewHackingStrategy(HackingStrategy *new_hs) {
    if (researcher.hacking_strategies.empty()) {
      researcher.hacking_strategies.resize(1);
    }
    //            researcher.hacking_strategies.back().push_back(new_hs);

//    researcher.is_hacker = true;
    return *this;
  }

  /**
   Prepare a set of hacking strategies groups by populating each group from
   the given `hacking_strategies_pool`

   \param hacking_strategies_pool A set of hacking strategy methods use to
   prepare researcher's hacking startegies
   \param n_group The number of hacking strategies groups
   \param m_strategies The number of hacking startegies in each group
   \return Return an instance of itself where hacking_strategies has been
   initialized accordingly.
   */
  ResearcherBuilder &
  chooseHackingStrategies(std::vector<HackingMethod> hacking_strategies_pool,
                          int n_group, int m_strategies) {

    //            for (auto &group : researcher.hacking_strategies) {
    //
    //                for (int i = 0; i < m_strategies; ++i) {
    //                    // TEST ME!
    //                    // TODO: I'm not sure if I want to have this builder
    ////                    auto h_method =
    /// enum_cast<HackingMethod>(Random::get<int>(0,
    /// hacking_strategies_pool.size() - 1));
    //                    // I think this should use the index from the list!
    //                    Test it!
    //// group.push_back(HackingStrategy::build(h_method.value()));
    //                }
    //
    //            }

//    researcher.is_hacker = true;
    return *this;
  };

  ///
  /// Constructs `n_group`'s of hacking strategies, each consisting of maximum
  /// `m_strategies`'s or steps. Each startegy is being selected randomly
  /// between all available strategies.
  ///
  /// \param      n_group       Number of groups of hacking strategies
  /// \param      m_strategies  Number of hacking strategies in each group
  /// \return     Return an instance of itself where hacking_strategies has been
  ///             initialized accordingly.
  ///
  ResearcherBuilder &pickRandomHackingStrategies(int n_group, int m_method) {

    researcher.isHacker();
    researcher.hacking_strategies.resize(n_group);

    //    for (auto &group : researcher.hacking_strategies) {
    //
    //      for (int i = 0; i < m_method; ++i) {
    //                            auto h_method =
    //                            enum_cast<HackingMethod>(Random::get<int>(0,
    //                            enum_count<HackingMethod>() - 1));
    //                            group.push_back(HackingStrategy::build(h_method.value()));
    //      }
    //    }

//    researcher.is_hacker = true;
    return *this;
  };

  ///
  /// Build and return a new Researcher.
  ///
  /// \note       Be aware that this needs to be called after you set all
  /// aspects of
  ///             the Researcher
  ///
  /// \return     A new Researcher
  ///
  Researcher build() {

    // TODO: Check if everything is setup carefully. I think I need several
    // flags like is_hacking_strat_set, is_hacker, etc. to make sure that the
    // Researcher is completely being constructed.

    return std::move(researcher);
  };

  //        operator Researcher() const {
  //            return std::move(researcher);
  //        }
};

} // namespace sam

#endif // SAMPP_RESEARCHER_H
