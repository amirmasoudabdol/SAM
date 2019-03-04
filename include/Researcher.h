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

//    TestStrategies* testStrategy;
    
    bool isHacker = false;
    HackingStyle hackingStyle = onCopy;
    std::vector<HackingStrategy*> hackingStrategies;
	std::vector<Experiment> experimentsList;
	std::vector<Submission> submissionsList;

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

    void research();
    
    ResearcherPreference selectionPref = ResearcherPreference::PreRegisteredOutcome;      ///< By default, a researcher always prefer to return the pre-registered result
    
    // std::string decisionStrategy = "asap";
    DecisionStrategy* decisionStrategy;
    void setDecisionStrategy(DecisionStrategy* d);
    
//    DecisionStrategy* finalDecision;
    // The `finalDecision` can be used alongside the `decisionStrategy`. While
    // `decisionStrategy` is being used to construct a submission from an experiment,
    // the `finalDecision` can be used to travese between all other submisions, and
    // pick the ultimate best. Having two decision strategy allow for more detail
    // implementation.
	
    // This could be renamed to something like, selectThePreferedSubmission()
    void prepareTheSubmission();
    void submitToJournal();

	void setJournal(Journal* j);

	// Reset the researcher state!
	void rest();


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
                this->_hackingStrategies.push_back(HackingStrategy::buildHackingMethod(item));
            }
        }
        return *this;
    };


    Builder& setExperimentSetup(ExperimentSetup);
    Builder& setExperiment(Experiment);
    Builder& setJournal(Journal j){
        this->_journal = &j;
        return *this;
    };
    Builder& setDecisionStrategy(DecisionStrategy);
    Builder& setHackingStrategy(HackingStrategy);
    Builder& setHackingStrategy(std::vector<HackingStrategy*>);
    
    Builder& chooseHackingStrategies(std::vector<HackingStrategy>);

    Researcher build() {
        return Researcher(_experiment, _journal, _decisionStrategy, _hackingStrategies);
    };
};

#endif //SAMPP_RESEARCHER_H
