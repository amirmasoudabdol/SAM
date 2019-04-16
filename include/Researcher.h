//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_RESEARCHER_H
#define SAMPP_RESEARCHER_H

#include <vector>
#include "Experiment.h"
#include "TestStrategy.h"
#include "SubmissionRecord.h"
#include "HackingStrategy.h"
#include "Journal.h"
#include "DecisionStrategy.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class Researcher {

public:

    class Builder;

    Experiment* experiment;
	Journal* journal;
    DecisionStrategy* decisionStrategy;
    std::vector<std::vector<HackingStrategy*>> hackingStrategies;
    bool is_hacker = false;
    
    Submission submissionRecord;

    Researcher(json& config);

    Researcher(Experiment* e) {
        experiment = e;
    };
    
    Researcher(Experiment* e,
                    Journal* j,
                    DecisionStrategy* ds,
                    std::vector<std::vector<HackingStrategy*>> hs,
                    bool ish) :
    experiment(e),
    journal(j),
    decisionStrategy(ds),
    hackingStrategies(hs),
    is_hacker(ish)
    { };

    // This probably needs to be a class of itself as well
    void registerAHackingStrategy(HackingStrategy* h);
    void hack();

    void prepareResearch();
    void performResearch();
    void publishResearch();
    
    DecisionPreference selectionPref = DecisionPreference::PreRegisteredOutcome;      ///< By default, a researcher always prefer to return the pre-registered result
    
    void setDecisionStrategy(DecisionStrategy* d);
	
    // This could be renamed to something like, selectThePreferedSubmission()
    void prepareTheSubmission();
    void submitToJournal();

	void setJournal(Journal* j);


};


class Researcher::Builder {
private:
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
            for (auto &set : config["ResearcherParameters"]["p-hacking-methods"]) {
                
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
    Builder& setDataStrategy(DataStrategy *dgs){
        this->experiment->dataStrategy = dgs;
        return *this;
    }
    Builder& setTestStrategy(TestStrategy *ts){
        this->experiment->testStrategy = ts;
        return *this;
    };
    Builder& setJournal(Journal j){
        this->journal = &j;
        return *this;
    };
    Builder& setDecisionStrategy(DecisionStrategy *ds);
    Builder& setHackingStrategy(HackingStrategy *hs);
    Builder& setHackingStrategy(std::vector<std::vector<HackingStrategy*>>);
    
    Builder& chooseHackingStrategies(std::vector<HackingStrategy>);

    Researcher build() {
        return Researcher(experiment, journal, decision_strategy, hacking_strategies, is_hacker);
    };
};

#endif //SAMPP_RESEARCHER_H
