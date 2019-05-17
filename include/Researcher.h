//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_RESEARCHER_H
#define SAMPP_RESEARCHER_H

#include <vector>
#include <random>

#include "Experiment.h"
#include "TestStrategy.h"
#include "Submission.h"
#include "HackingStrategy.h"
#include "Journal.h"
#include "DecisionStrategy.h"

#include "nlohmann/json.hpp"

//#include "ResearcherBuilder.h"


namespace sam {

    using json = nlohmann::json;
    
    class ResearcherBuilder;

    class Researcher {
        // Making the ResearcherBuilder a friend class in order to give it access to private
        // memebers of the Researcher. At the moment it's only `name` but I'll encapsulate
        // more variables as the time goes.
        friend class ResearcherBuilder;

        //! An arbitrary name for the Researcher
        std::string name;
        
        /**
         Default constructor of the Researcher.
         
         @note This is defined `private` because I want force the user to use
         the `create(name)` method and therefore delegate the construction to
         the ResearcherBuilder.
         */
        Researcher() = default;
        
    public:

        /**
         Starts the Researcher build process. Use this to build a new instance
         of the Researcher.

         @param name The researcher name
         @return An instance of ResearcherBuilder.
         */
        static ResearcherBuilder create(std::string name);
        
        Experiment* experiment;
        Journal* journal;
        DecisionStrategy* decision_strategy;
        std::vector<std::vector<HackingStrategy*>> hacking_strategies;
        
        bool is_hacker = false;
        bool isHacker() const {
            return is_hacker;
        }
        
        //! A Submission record that Researcher is going to submit to the Journal
        Submission submission_record;

        void hack();

        void prepareResearch();
        void performResearch();
        void publishResearch();
        
        // This could be renamed to something like, selectThePreferedSubmission()
        void prepareTheSubmission();
        void submitToJournal();
        
        /**
         Set the decisionStrategy of the researcher.

         @param d The pointer to a Decision Strategy
         */
        void setDecisionStrategy(DecisionStrategy* d) {
            decision_strategy = d;
        }
        
        /**
         Set the experiment. This can be used to setup several researchers with one
         experiment.
         
         @param e The pointer to an Experiment
         */
        void setExperiment(Experiment* e) {
            experiment = e;
        };
        
        /**
         @brief      Set the Jouranl
        
         @param      j     The pointer to a Journal instance
         */
        void setJournal(Journal* j) {
            journal = j;
        };
        
    };
    
    /**
     ResearcherBuilder class for Researcher. This takes care of eveyrthing and return a
     fully initialized Researcher after calling `.build()` method.
     */
    class ResearcherBuilder {
        
        Researcher researcher;
        
        json config;
        bool build_from_config = false;        
        
    public:
        
        ResearcherBuilder() { };
        
        ResearcherBuilder(std::string name) {
            researcher.name = name;
        };
        
        /**
         Build a researcher entirely based on the given config file. This is
         not the best implementation still but I think it's more readable and
         reasonable for some usecases.
         
         @param config A JSON object
         @return Return an instance of itself
         */
        ResearcherBuilder& fromConfigFile(json& config) {
            
            this->config = config;
            
//            auto setup = ExperimentSetup::create().fromConfigFile(config).build();
            
            researcher.experiment = new Experiment(config);
            
            researcher.journal = new Journal(config["JournalParameters"]);
            
            researcher.decision_strategy = DecisionStrategy::build(config["ResearcherParameters"]["decision-strategy"]);
            
            researcher.is_hacker = config["ResearcherParameters"]["is-phacker"];
            if (researcher.is_hacker){
                for (auto &set : config["ResearcherParameters"]["hacking-strategies"]) {
                    
                    researcher.hacking_strategies.push_back({});
                    
                    for (auto &item : set) {
                        
                        researcher.hacking_strategies.back()
                        .push_back(HackingStrategy::build(item));
                        
                    }
                    
                }
            }
            
            build_from_config = true;
            
            return *this;
        }

        ResearcherBuilder& setExperiment(Experiment *expr) {
            researcher.experiment = expr;
            return *this;
        }
        
        ResearcherBuilder& setExperimentSetup(ExperimentSetup es) {
            researcher.experiment = new Experiment(es);
            return *this;
        };
        
        ResearcherBuilder& setResearcherPreference(DecisionPreference pref){
            researcher.decision_strategy->selectionPref = pref;
            return *this;
        };
        

        ResearcherBuilder& createDecisionStrategy(const DecisionStrategy::DecisionStrategyParameters &dsp) {
            researcher.decision_strategy = DecisionStrategy::build(dsp);
            return *this;
        };

        ResearcherBuilder& createJournal(Journal::JournalParameters &jp, 
                                         SelectionStrategy::SelectionStrategyParameters &ssp) {
            researcher.journal = new Journal(jp, ssp);
            return *this;
        }

        ResearcherBuilder& createExperiment(ExperimentSetup es) {
            researcher.experiment = new Experiment(es);
            return *this;
        }

        ResearcherBuilder& setDataStrategy(std::shared_ptr<DataStrategy> ds) {
            researcher.experiment->data_strategy = ds;
            return *this;
        }
        ResearcherBuilder& setTestStrategy(std::shared_ptr<TestStrategy> &ts) {
            researcher.experiment->test_strategy = ts;
            return *this;
        };
        ResearcherBuilder& setJournal(Journal *j) {
            researcher.journal = j;
            return *this;
        };
        ResearcherBuilder& setJournalSelectionStrategy(SelectionStrategy *ss) {
            researcher.journal->setSelectionStrategy(ss);
            return *this;
        };
        ResearcherBuilder& setDecisionStrategy(DecisionStrategy *ds) {
            researcher.decision_strategy = ds;
            return *this;
        };
        ResearcherBuilder& setHackingStrategy(HackingStrategy *hs);
        ResearcherBuilder& setHackingStrategy(std::vector<std::vector<HackingStrategy*>>);
        
        
        ResearcherBuilder& addHackingStrategyGroup(std::vector<HackingStrategy* > hsg) {
            researcher.hacking_strategies.push_back(hsg);
            return *this;
        }
        
        ResearcherBuilder& addNewHackingStrategy(HackingStrategy *new_hs) {
            if (researcher.hacking_strategies.empty()){
                researcher.hacking_strategies.resize(1);
            }
            researcher.hacking_strategies.back().push_back(new_hs);
            
            return *this;
        }
        
        /**
         Prepare a set of hacking strategies groups by populating each group from
         the given `hacking_strategies_pool`
         
         @param hacking_strategies_pool A set of hacking strategy methods use to
         prepare researcher's hacking startegies
         @param n_group The number of hacking strategies groups
         @param m_strategies The number of hacking startegies in each group
         @return Return an instance of itself where hacking_strategies has been
         initialized accordingly.
         */
        ResearcherBuilder& chooseHackingStrategies(std::vector<HackingMethod> hacking_strategies_pool, int n_group, int m_strategies) {
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> uniform(0, hacking_strategies_pool.size() - 1);
            
            for (auto &group : researcher.hacking_strategies) {
                
                for (int i = 0; i < m_strategies; i++) {
                    group.push_back(HackingStrategy::build(static_cast<HackingMethod>(uniform(gen))));
                }
                
            }
            
            return *this;
        };
        
        
        /**
         Constructs `n_group`'s of hacking strategies, each consisting of maximum
         `m_strategies`'s or steps. Each startegy is being selected randomly
         between all available strategies.
         
         @param n_group Number of groups of hacking strategies
         @param m_strategies Number of hacking strategies in each group
         @return Return an instance of itself where hacking_strategies has been
         initialized accordingly.
         */
        ResearcherBuilder& pickRandomHackingStrategies(int n_group, int m_method) {
            
            researcher.isHacker();
            researcher.hacking_strategies.resize(n_group);
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> uniform(1, static_cast<int>(HackingMethod::N_HACKING_METHODS) - 1);
            
            for (auto &group : researcher.hacking_strategies) {
                
                for (int i = 0; i < m_method; i++) {
                    group.push_back(HackingStrategy::build(static_cast<HackingMethod>(uniform(gen))));
                }
                
            }
            
            return *this;
        };
        
        /**
         Build and return a new Researcher.
         
         @note Be aware that this needs to be called after you set all aspects
         of the Researcher
         
         @return A new Researcher
         */
        Researcher build() {
            
            // TODO: Check if everything is setup carefully. I think I need several
            // flags like is_hacking_strat_set, is_hacker, etc. to make sure that the
            // Researcher is completely being constructed.
            
            return researcher;
            
        };
        
        operator Researcher() const {
            return researcher;
        }
    };


}

#endif //SAMPP_RESEARCHER_H
