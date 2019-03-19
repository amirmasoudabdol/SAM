//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_RESEARCHER_H
#define SAMPP_RESEARCHER_H

#include <vector>
#include "Experiment.h"
#include "TestStrategy.h"
#include "SubmissionRecord.h"
#include "HackingStrategies.h"
#include "Journal.h"
#include "DecisionStrategy.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

enum HackingStyle {
    onOrig,
    onCopy
};

class Researcher {

public:

    class Builder;

    Experiment* experiment;
	Journal* journal;
    DecisionStrategy* decisionStrategy;
    std::vector<HackingStrategy*> hackingStrategies;
    bool isHacker = false;
    
    HackingStyle hackingStyle = onCopy;

    Submission submissionRecord;

    Researcher(json& config);

    Researcher(Experiment* e) {
        experiment = e;
    };
    
    Researcher(Experiment* e,
                    Journal* j,
                    DecisionStrategy* ds,
                    std::vector<HackingStrategy*> hs,
                    bool ish) :
    experiment(e),
    journal(j),
    decisionStrategy(ds),
    hackingStrategies(hs),
    isHacker(ish)
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


private:
	int _pvalue_min_inx;
	int _max_effect_inx;
	int _selected_outcome_inx = 0;
	Submission _create_submission_record(int inx);

};


class Researcher::Builder {
private:
    json _config;
    

    ExperimentSetup* _experiment_setup;
    Experiment* _experiment;
    Journal* _journal;
    
    bool _isHacker;
    std::vector<HackingStrategy*> _hackingStrategies;

    DecisionStrategy* _decisionStrategy;

    DecisionPreference _researcherPreference;


public:

    Builder() = default;

    Builder& setConfig(json& config) {
        this->_config = config;
        return *this;
    };
    
    Builder& makeExperimentSetup();
    
    Builder& makeExperiment() {
        this->_experiment = new Experiment(_config);
        return *this;
    };
    
    Builder& makeJournal() {
        this->_journal = new Journal(_config["Journal Parameters"]);
        return *this;
    };
    
    Builder& makeDecisionStrategy() {
        this->_decisionStrategy = DecisionStrategy::buildDecisionStrategy(_config["Researcher Parameters"]["--decision-strategy"]);
        return *this;
    };
    
    Builder& isHacker() {
        this->_isHacker = _config["Researcher Parameters"]["--is-phacker"];
        return *this;
    }
    
    Builder& makeHackingStrategies(){
        this->isHacker();
        if (this->_isHacker){
            for (auto &item : _config["Researcher Parameters"]["--p-hacking-methods"]) {
                // if (item["type"] == "Outcome Switching"){
                //     this->setResearcherPreference(stringToResearcherPreference.find(item["preference"])->second);
                // }else{
                    this->_hackingStrategies.push_back(HackingStrategy::buildHackingMethod(item));
                // }
            }
        }
        return *this;
    };


    Builder& setResearcherPreference(DecisionPreference pref){
        // TODO: This still needs to be feeded to the Researcher's constructor which doesn't support it yet.
        this->_researcherPreference = pref;
        return *this;
    };
    
    Builder& setExperimentSetup(ExperimentSetup);
    Builder& setExperiment(Experiment);
    Builder& setDataStrategy(DataGenStrategy *dgs){
        this->_experiment->dataStrategy = dgs;
        return *this;
    }
    Builder& setTestStrategy(TestStrategy *ts){
        this->_experiment->testStrategy = ts;
        return *this;
    };
    Builder& setJournal(Journal j){
        this->_journal = &j;
        return *this;
    };
    Builder& setDecisionStrategy(DecisionStrategy);
    Builder& setHackingStrategy(HackingStrategy);
    Builder& setHackingStrategy(std::vector<HackingStrategy*>);
    
    Builder& chooseHackingStrategies(std::vector<HackingStrategy>);

    Researcher build() {
        return Researcher(_experiment, _journal, _decisionStrategy, _hackingStrategies, _isHacker);
    };
};

#endif //SAMPP_RESEARCHER_H
