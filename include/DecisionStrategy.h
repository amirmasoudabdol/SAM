//
// Created by Amir Masoud Abdol on 2019-02-01.
//

#ifndef SAMPP_DECISIONSTRATEGY_H
#define SAMPP_DECISIONSTRATEGY_H

#include <vector>

#include "SubmissionRecord.h"
#include "Experiment.h"

enum ResearcherPreference {
    PreRegisteredOutcome,
    MinSigPvalue,
    MinPvalue,
    MaxSigEffect,
    MaxEffect,
    MinPvalueMaxEffect
};

class DecisionStrategy {

public:
    
    ResearcherPreference selectionPref;
    bool isStillHacking = true;
    
    int preRegGroup = 0;
    Submission finalSubmission;
    
    double alpha = 0.05;
    
    virtual Submission selectOutcome(Experiment& experiment) = 0;
    
    // These two are quite similar, and they are being used in the prepareForSubmission
    // routine where the researcher makes the final decision, in this way, based on how
    // aggressive his decision making is, he can choose between travesing all groups — all
    // the way back, through all hacking — or travese only between the submissions that he
    // has previously selected.
    // Check *Researcher* for more info on more complex setup
//    virtual Submission selectBetweenSubmission(std::vector<Submission>& submissions) = 0;
//    virtual Submission selectBetweenExperiments(std::vector<Experiment>& experiments) = 0;
    
    virtual void verdict(std::vector<Submission>&, std::vector<Experiment>&) = 0;
    
    Submission _select_Outcome(Experiment&);
};


class ImpatientDecisionMaker : public DecisionStrategy {

public:
    ImpatientDecisionMaker(int pre_registered_group,
                           double alpha,
                           ResearcherPreference selection_pref){
        preRegGroup = pre_registered_group;
        selectionPref = selection_pref;
    };
    
    Submission selectOutcome(Experiment& experiment) {
        return _select_Outcome(experiment);
    };
    
    void verdict(std::vector<Submission>& submissions, std::vector<Experiment>& experiments) {
        
        finalSubmission = submissions.back();
        
        if (submissions.back().pvalue < alpha) {
            isStillHacking = false;
        }else{
            isStillHacking = true;
        }
    }
    
};


class ReportPreregisteredGroup : public DecisionStrategy {

//    int _pre_registered_group = 0;

public:

    
    ReportPreregisteredGroup(int pre_registered_group){
        preRegGroup = pre_registered_group;
    };

    Submission selectOutcome(Experiment& experiment) {
        return Submission(experiment, preRegGroup);
    };
    
    virtual void verdict(std::vector<Submission>& submissions, std::vector<Experiment>& experiments) {
        isStillHacking = true;
    }

//    Submission selectBetweenSubmission(std::vector<Submission>& submissions){
//
//        return submissions.back(); // FIXME: For now this is alright!
//    };
//    Submission selectBetweenExperiments(std::vector<Experiment>& experiments){
//        return Submission();
//    };
    
};

#endif //SAMPP_DECISIONSTRATEGY_H
