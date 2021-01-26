//===-- Researcher.cpp - Researcher Deceleration --------------------------===//
//
// Part of the SAM Project
// Created by Amir Masoud Abdol on 2019-01-25.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// This file contains the deceleration of the Researcher class which handles 
/// the majority of the work in SAM from initiating the experiment, to running 
/// it and submitting it to the Journal.
///
//===----------------------------------------------------------------------===// 
///
/// @defgroup   AbstractBuilders Abstract Factory Builders
/// @brief      List of Abstract Factory Builder classes
///
/// Description to come!
///

#ifndef SAMPP_RESEARCHER_H
#define SAMPP_RESEARCHER_H

#include "Experiment.h"
#include "HackingProbabilityStrategy.h"
#include "HackingStrategy.h"
#include "Journal.h"
#include "ResearchStrategy.h"
#include "Submission.h"

using Random = effolkronium::random_static;

namespace sam {

class ResearcherBuilder;

/// A helper typedef representing selection → decision sequence
using HackingWorkflow =
    std::vector<std::vector<std::variant<std::shared_ptr<HackingStrategy>,
                                         PolicyChain, PolicyChainSet>>>;

///
/// @brief      This class describes a researcher
///
/// Researcher is the main player of the game. It has access to _almost_
/// everything and it works with several other components to conduct and
/// evaluate the research, and prepare it to be submitted to the Journal.
///
class Researcher {

  friend class ResearcherBuilder;

  //! An arbitrary (and optional) name of the Researcher
  std::string name;

  //! Number of hacking strategies to be chosen from the given list of
  //! strategies
  size_t n_hacks;

  //! Indicates whether the researcher is going to randomize its hacking arsenal
  //! for every new research
  bool reselect_hacking_strategies_after_every_simulation{false};

  //! Indicates the order in which hacking strategies are going to be selected
  //! from the list of given hacking strategies.
  //!
  //! @note This has effect only if researcher decides to select fewer 
  //! strategies than the number of hacking strategies provided, 
  //! #n_hacks < #original_workflow.size()
  std::string hacking_selection_priority;

  //! Indicates the execution order of the selected/given hacking strategies
  std::string hacking_execution_order;

  //! Indicates the probability of committing to the submission process given 
  //! the availability of submissions
  double submission_probability{1};

  //! A list of satisfactory submissions collected by the researcher at the end
  //! of each replications.
  SubmissionPool replication_submissions;

  std::optional<SubmissionPool> hacked_submissions;
  std::optional<SubmissionPool> stashed_submissions;
  std::optional<SubmissionPool> candidate_submissions;

  //! Indicates the probability of a Researcher _deciding_ to _go for_ hacking 
  //! an Experiment.
  //! 
  //! This being a Parameter allows it to be randomized at each run if necessary
  //! 
  Parameter<double> probability_of_being_a_hacker;

  //! Indicates the probability of a Researcher _actually applying_ a chosen
  //! hacking strategy.
  //! 
  //! This can have any of the given numbers:
  //! - A fixed value
  //! - A string indicating whether the decision should be make based on the 
  //! defensibly or prevalence of the method
  //! - A univariate distribution returning a value between 0 and 1.
  //! - One of the HackingProbabilityStrategy classes, which will be used by the
  //! researcher to base her decision based on characteristic of individual 
  //! experiment.
  std::variant<double, std::string, UnivariateDistribution,
               std::unique_ptr<HackingProbabilityStrategy>>
      probability_of_committing_a_hack;

  //! Indicates whether the researcher performs any pre-processing on the method
  bool is_pre_processing{false};

public:
  //! List of hacking strategies that are going to be applied on the experiment
  //! during the pre-processing stage
  std::vector<std::unique_ptr<HackingStrategy>> pre_processing_methods;

  //! Researcher's Experiment
  std::unique_ptr<Experiment> experiment;

  //! Researcher's Journal of choice!
  std::shared_ptr<Journal> journal;

  //! Researcher's Research Strategy
  std::unique_ptr<ResearchStrategy> research_strategy;

  //! Original set of hacking strategies and their Selection→Decision sequences
  HackingWorkflow original_workflow;

  //! Researcher's hacking workflow. This is a subset of the #original_workflow,
  //! as it is being filtered and rearranged by various factors during the
  //! initialization.
  HackingWorkflow hacking_workflow;

  /// This doesn't do anything! But it should! At the moment, I'm relying on the
  /// ResearcherBuilder to construct the Researcher but this has to change
  Researcher() = default;

  ///
  /// Starts the Researcher build process. Use this to build a new instance of
  /// the Researcher.
  ///
  /// @param      name  The researcher name
  /// @return     An instance of ResearcherBuilder.
  ///
  static ResearcherBuilder create(std::string name);

  /// Applies the pro-processing methods on the Experiment
  void preProcessData();

  /// Determines whether the Researcher is a hacker
  bool isHacker();

  /// Determines whether the Researcher will commit to the given hack
  bool isCommittingToTheHack(HackingStrategy *hs);

  /// Perform the Research
  void research();

  /// Applies the HackingWorkflow on the Experiment
  std::optional<SubmissionPool> hackTheResearch();

  /// Randomizes the internal state of the Researcher
  void randomizeParameters();

  /// Evaluating the candidates and submitting them to the Journal
  void
  submitTheResearch(const std::optional<std::vector<Submission>> &subs);

  /// A helper function for re-computing all statistics at once
  void computeStuff() const {
    experiment->calculateStatistics();
    experiment->calculateEffects();
    experiment->calculateTests();

    spdlog::trace("{}", *experiment);
  }

  /// Resets the internal state of the Researcher
  void reset() {
    experiment->reset();
    research_strategy->reset();
    replication_submissions.clear();
    hacked_submissions.reset();
    stashed_submissions.reset();
    candidate_submissions.reset();
  }

private:
  /// Re-orders the given workflow according the given priority
  static void reorderHackingStrategies(HackingWorkflow &hw, std::string &priority);

public:
  //--- Builder Related Methods

  ///
  /// Set the researchStrategy of the researcher.
  ///
  /// @param      d     The pointer to a Research Strategy
  ///
  /// @note       Researcher owns its research strategy that's why I move the
  ///             pointer.
  ///
  /// @todo       I think I need to do the `std::move` when I'm calling the
  /// function
  ///             not inside it
  ///
  //  void setResearchStrategy(std::unique_ptr<ResearchStrategy> ds) {
  //    research_strategy = std::move(ds);
  //  }

  ///
  /// Set the experiment. This can be used to setup several researchers with one
  /// experiment.
  ///
  /// @param      e     The pointer to an Experiment
  ///
  //  void setExperiment(Experiment *e) { experiment = e; };

  ///
  /// @brief      Set the Journal
  ///
  /// @param      j     The pointer to a Journal instance
  ///
  //  void setJournal(Journal *j) { journal = j; };
};

///
/// ResearcherBuilder class for Researcher. This takes care of everything and
/// return a fully initialized Researcher after calling `.build()` method.
///
/// @ingroup AbstractBuilders
class ResearcherBuilder {

  Researcher researcher;

  json config;
  bool build_from_config = false;

public:
  ResearcherBuilder() = default;
  ;

  ResearcherBuilder(std::string name) { researcher.name = name; };

  ///
  /// Build a researcher entirely based on the given config file. This is not
  /// the best implementation still but I think it's more readable and
  /// reasonable for some use-cases.
  ///
  /// @param      config  A JSON object
  /// @return     Return an instance of itself
  ///
  /// @todo This needs to be split into different pieces
  ResearcherBuilder &fromConfigFile(json &config) {

    this->config = config;

    /// @todo I can technically replace all these direct calls with calls
    /// to their counterpart in the Builder!

    researcher.experiment =
        std::make_unique<Experiment>(config["experiment_parameters"]);

    config["journal_parameters"]["save_meta"] =
        config["simulation_parameters"]["save_meta"];
    config["journal_parameters"]["save_pubs_per_sim_summaries"] =
        config["simulation_parameters"]["save_pubs_per_sim_summaries"];
    config["journal_parameters"]["save_overall_summaries"] =
        config["simulation_parameters"]["save_overall_summaries"];

    config["journal_parameters"]["output_path"] =
        config["simulation_parameters"]["output_path"];
    config["journal_parameters"]["output_prefix"] =
        config["simulation_parameters"]["output_prefix"];
    researcher.journal =
        std::make_shared<Journal>(config["journal_parameters"]);

    researcher.research_strategy = ResearchStrategy::build(
        config["researcher_parameters"]["research_strategy"]);

    researcher.is_pre_processing =
        config["researcher_parameters"]["is_pre_processing"];
    if (researcher.is_pre_processing) {
      for (auto &item :
           config["researcher_parameters"]["pre_processing_methods"]) {

        researcher.pre_processing_methods.push_back(
            HackingStrategy::build(item));
      }
    }

    researcher.probability_of_being_a_hacker = Parameter<double>(
        config["researcher_parameters"]["probability_of_being_a_hacker"], 1);

    auto prob_of_committing_a_hack =
        config["researcher_parameters"]["probability_of_committing_a_hack"];
    switch (prob_of_committing_a_hack.type()) {
    case nlohmann::detail::value_t::number_integer:
    case nlohmann::detail::value_t::number_unsigned:
    case nlohmann::detail::value_t::number_float:
      researcher.probability_of_committing_a_hack =
          prob_of_committing_a_hack.get<double>();
      break;
    case nlohmann::detail::value_t::string:
      researcher.probability_of_committing_a_hack =
          prob_of_committing_a_hack.get<std::string>();
      break;
    case nlohmann::detail::value_t::object: {
      if (prob_of_committing_a_hack.contains("dist")) {
        researcher.probability_of_committing_a_hack =
            makeUnivariateDistribution(prob_of_committing_a_hack);
      } else {
        researcher.probability_of_committing_a_hack =
            HackingProbabilityStrategy::build(prob_of_committing_a_hack);
      }
    } break;
    default:
      researcher.probability_of_committing_a_hack = 0.;
      break;
    }

    // Parsing Hacking Strategies
    researcher.hacking_workflow.resize(
        config["researcher_parameters"]["hacking_strategies"].size());

    for (int h{0}; h < researcher.hacking_workflow.size(); ++h) {

      // Basically there are no hacking strategies defined
      if (config["researcher_parameters"]["hacking_strategies"][0] == "") {
        break;
      }

      auto &item = config["researcher_parameters"]["hacking_strategies"][h];

      researcher.hacking_workflow[h].push_back(HackingStrategy::build(item[0]));

      researcher.hacking_workflow[h].push_back(
          PolicyChainSet{item[1].get<std::vector<std::vector<std::string>>>(),
                         researcher.research_strategy->lua});

      researcher.hacking_workflow[h].push_back(PolicyChain{
          item[2].get<std::vector<std::string>>(), PolicyChainType::Decision,
          researcher.research_strategy->lua});
    }

    /// Copying the original list for later shuffling!
    researcher.original_workflow = researcher.hacking_workflow;

    if (config["researcher_parameters"].contains("randomize_strategies")) {
      researcher.reselect_hacking_strategies_after_every_simulation =
          config["researcher_parameters"]["randomize_strategies"].get<bool>();
    }

    researcher.n_hacks = researcher.hacking_workflow.size();
    if (config["researcher_parameters"].contains("n_hacks")) {
      researcher.n_hacks =
          std::min(researcher.hacking_workflow.size(),
                   config["researcher_parameters"]["n_hacks"].get<size_t>());
    }

    //! Selecting between hacking
    if (researcher.n_hacks < researcher.hacking_workflow.size() and
        config["researcher_parameters"].contains(
            "hacking_selection_priority")) {
      researcher.hacking_selection_priority =
          config["researcher_parameters"]["hacking_selection_priority"]
              .get<std::string>();

      /// Reordering hacking strategies based on selection priority
      researcher.reorderHackingStrategies(
          researcher.hacking_workflow, researcher.hacking_selection_priority);

      /// Selecting only n_hacks of those
      researcher.hacking_workflow.resize(researcher.n_hacks);
    }

    //! Setting the execution order
    if (config["researcher_parameters"].contains("hacking_execution_order")) {
      researcher.hacking_execution_order =
          config["researcher_parameters"]["hacking_execution_order"]
              .get<std::string>();

      /// Reorder hacking strategies based on the preferred execution order
      researcher.reorderHackingStrategies(researcher.hacking_workflow,
                                          researcher.hacking_execution_order);
    }

    /// Sorting the selected hacking strategies based on their stage
    /// The stable_sort has been used because I'd like to keep the given
    /// order in previous stages
    std::stable_sort(
        researcher.hacking_workflow.begin(), researcher.hacking_workflow.end(),
        [&](auto &h1, auto &h2) {
          return std::get<0>(h1[0])->stage() < std::get<0>(h2[0])->stage();
        });

    /// Indicate what percentage of Researcher's work is going to be submitted
    if (config["researcher_parameters"].contains("submission_probability")) {
      researcher.submission_probability =
          config["researcher_parameters"]["submission_probability"]
              .get<double>();
    }

    build_from_config = true;

    return *this;
  }

  ////////////////////////////
  /// CREATEING NEW OBJECT ///
  ////////////////////////////

  ///
  /// @brief      Create a new ResearchStrategy for the researcher based on the
  /// given configuration.
  ///
  ResearcherBuilder &createResearchStrategy(json &ds) {
    researcher.research_strategy = ResearchStrategy::build(ds);
    return *this;
  }

  ///
  /// @brief      Create a new Journal for the researcher based on the given
  ///             configuration.
  ///
  /// @note       The configuration needs to include information about the
  ///             SelectionStrategy as well.
  ///
  //  ResearcherBuilder &createJournal(json &journal_config) {
  //    researcher.journal = new Journal(journal_config);
  //    return *this;
  //  }

  ///
  /// @brief      Create a new Experiment based on the given ExperimentSetup.
  ///
  /// @note       : This assumes that the experiment setup is correctly
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

  //        ResearcherBuilder& setResearchStrategy(const Research Strategyds){
  //            researcher.research_strategy = ds;
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

  // I'm not sure if I should leave these here
  ResearcherBuilder &setDataStrategy(std::shared_ptr<DataStrategy> ds) {
    researcher.experiment->data_strategy = ds;
    return *this;
  }

  // I'm not sure if I should leave these here
  ResearcherBuilder &setTestStrategy(std::shared_ptr<TestStrategy> &ts) {
    researcher.experiment->test_strategy = ts;
    return *this;
  };

  //  ResearcherBuilder &setJournal(Journal *j) {
  //    researcher.journal = j;
  //    return *this;
  //  };

  ResearcherBuilder &
  setJournalSelectionStrategy(std::unique_ptr<ReviewStrategy> ss) {

    researcher.journal->setSelectionStrategy(std::move(ss));
    return *this;
  };

  ResearcherBuilder &setResearchStrategy(std::unique_ptr<ResearchStrategy> ds) {
    //    researcher.setResearchStrategy(std::move(ds));
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

  //  ResearcherBuilder &addNewHackingStrategy(HackingStrategy *new_hs) {
  //    if (researcher.hacking_strategies.empty()) {
  //      researcher.hacking_strategies.resize(1);
  //    }
  //    //            researcher.hacking_strategies.back().push_back(new_hs);
  //
  //    //    researcher.is_hacker = true;
  //    return *this;
  //  }

  /**
   Prepare a set of hacking strategies groups by populating each group from
   the given `hacking_strategies_pool`

   @param hacking_strategies_pool A set of hacking strategy methods use to
   prepare researcher's hacking strategies
   @param n_group The number of hacking strategies groups
   @param m_strategies The number of hacking strategies in each group
   @return Return an instance of itself where hacking_strategies has been
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
  /// `m_strategies`'s or steps. Each strategy is being selected randomly
  /// between all available strategies.
  ///
  /// @param      n_group       Number of groups of hacking strategies
  /// @param      m_strategies  Number of hacking strategies in each group
  /// @return     Return an instance of itself where hacking_strategies has been
  ///             initialized accordingly.
  ///
  //  ResearcherBuilder &pickRandomHackingStrategies(int n_group, int m_method)
  //  {
  //
  //    researcher.isHacker();
  //    researcher.hacking_strategies.resize(n_group);
  //
  //    //    for (auto &group : researcher.hacking_strategies) {
  //    //
  //    //      for (int i = 0; i < m_method; ++i) {
  //    //                            auto h_method =
  //    // enum_cast<HackingMethod>(Random::get<int>(0,
  //    //                            enum_count<HackingMethod>() - 1));
  //    // group.push_back(HackingStrategy::build(h_method.value()));
  //    //      }
  //    //    }
  //
  //    //    researcher.is_hacker = true;
  //    return *this;
  //  };

  ///
  /// Build and return a new Researcher.
  ///
  /// @note       Be aware that this needs to be called after you set all
  /// aspects of
  ///             the Researcher
  ///
  /// @return     A new Researcher
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
