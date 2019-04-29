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

namespace sam {

    using json = nlohmann::json;

    class Researcher {

    public:

        class Builder;

        Experiment* experiment;

        //! A pointer to the Journal used by researcher for submitting
        //! the submission or experiment
        Journal* journal;
        DecisionStrategy* decision_strategy;
        std::vector<std::vector<HackingStrategy*>> hacking_strategies;
        bool is_hacker = false;
        
        //! A Submission record that Researcher is going to submit to the Journal
        Submission submission_record;
        
        Researcher();

        Researcher(json& config);
        
        Researcher(Experiment* e,
                        Journal* j,
                        DecisionStrategy* ds,
                        std::vector<std::vector<HackingStrategy*>> hs,
                        bool ish) :
        experiment(e),
        journal(j),
        decision_strategy(ds),
        hacking_strategies(hs),
        is_hacker(ish)
        { };

        void hack();

        void prepareResearch();
        void performResearch();
        void publishResearch();
        
        //! By default, a researcher always prefer to return the pre-registered result
        DecisionPreference selection_pref = DecisionPreference::PreRegisteredOutcome;
        
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
     Builder class for Researcher. This takes care of eveyrthing and return a
     fully initialized Researcher after calling `.build()` method.
     */
    class Researcher::Builder {

        json config;
        
        ExperimentSetup* experiment_setup;
        Experiment* experiment;
        Journal* journal;
        
        bool is_hacker;
        std::vector<std::vector<HackingStrategy*>> hacking_strategies;

        DecisionStrategy* decision_strategy;

        DecisionPreference researcher_preference;


    public:

        Builder() = default;

        Builder& setConfig(json& config) {
            this->config = config;
            return *this;
        };
        
        
        /**
         Build a researcher entirely based on the given config file. This is
         not the best implementation still but I think it's more readable and
         reasonable for some usecases.

         @param config A JSON object
         @return Return an instance of itself
         */
        Builder& makeResearcherFromConfig(json& config) {
            this->config = config;
            this->experiment = new Experiment(config);
            this->journal = new Journal(config["JournalParameters"]);
            this->decision_strategy = DecisionStrategy::build(config["ResearcherParameters"]["decision-strategy"]);
            this->is_hacker = config["ResearcherParameters"]["is-phacker"];
            if (this->is_hacker){
                for (auto &set : config["ResearcherParameters"]["hacking-strategies"]) {
                    
                    this->hacking_strategies.push_back({});
                    
                    for (auto &item : set) {
                        
                        this->hacking_strategies.back().push_back(HackingStrategy::build(item));
                        
                    }
                    
                }
            }
            return *this;
        }
        
        // Experiment's constructor also prepares the ExperimentSetup!
        Builder& makeExperimentSetup();
        
        Builder& makeExperiment() {
            this->experiment = new Experiment(config);
            return *this;
        };
        
        Builder& makeJournal() {
            this->journal = new Journal(config["JournalParameters"]);
            return *this;
        };
        
        Builder& makeDecisionStrategy() {
            this->decision_strategy = DecisionStrategy::build(config["ResearcherParameters"]["decision-strategy"]);
            return *this;
        };
        
        Builder& isHacker() {
            this->is_hacker = config["ResearcherParameters"]["is-phacker"];
            return *this;
        }
        
        Builder& makeHackingStrategies(){
            this->isHacker();
            if (this->is_hacker){
                for (auto &set : config["ResearcherParameters"]["hacking-strategies"]) {
                    
                    this->hacking_strategies.push_back({});
                    
                    for (auto &item : set) {
                        
                        this->hacking_strategies.back().push_back(HackingStrategy::build(item));
                        
                    }
                    
                }
            }
            return *this;
        };


        Builder& setResearcherPreference(DecisionPreference pref){
            // TODO: This still needs to be feeded to the Researcher's constructor which doesn't support it yet.
            this->researcher_preference = pref;
            return *this;
        };
        
        Builder& setExperimentSetup(ExperimentSetup);
        Builder& setExperiment(Experiment);
        Builder& setDataStrategy(DataStrategy *dgs) {
            this->experiment->data_strategy = dgs;
            return *this;
        }
        Builder& setTestStrategy(TestStrategy *ts) {
            this->experiment->test_strategy = ts;
            return *this;
        };
        Builder& setJournal(Journal j) {
            this->journal = &j;
            return *this;
        };
        Builder& setJournalSelectionStrategy(SelectionStrategy *ss) {
            this->journal->setSelectionStrategy(ss);
            return *this;
        };
        Builder& setDecisionStrategy(DecisionStrategy *ds);
        Builder& setHackingStrategy(HackingStrategy *hs);
        Builder& setHackingStrategy(std::vector<std::vector<HackingStrategy*>>);
        
        
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
        Builder& chooseHackingStrategies(std::vector<HackingMethod> hacking_strategies_pool, int n_group, int m_strategies) {
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> uniform(0, hacking_strategies_pool.size() - 1);
            
            for (auto &group : hacking_strategies) {
                
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
        Builder& pickRandomHackingStrategies(int n_group, int m_method) {
            
            this->isHacker();
            this->hacking_strategies.resize(n_group);
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> uniform(1, static_cast<int>(HackingMethod::N_HACKING_METHODS) - 1);
            
            for (auto &group : hacking_strategies) {
                
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
            // TODO: I can make a researcher and set all its parameters using set* methods
            
            return Researcher(experiment, journal, decision_strategy, hacking_strategies, is_hacker);
        };
    };

}

#endif //SAMPP_RESEARCHER_H
