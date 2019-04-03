//
// Created by Amir Masoud Abdol on 2019-02-01.
//

#ifndef SAMPP_DECISIONSTRATEGY_H
#define SAMPP_DECISIONSTRATEGY_H

#include <vector>
#include <armadillo>

#include "SubmissionRecord.h"
#include "Experiment.h"

/**
 DecisionStage enum indicates on what stages of the _research_ the Researcher is
 making decision in. 
 
 */
enum class DecisionStage {
    Initial,
    WhileHacking,
    DoneHacking,
    Final
};

/**
 An enum class representing different prefeneces when it comse
 to selecting an outcome. The DecisionStrategy can choose to
 report Pre-registered outcome, or any other outcome based on
 certain criteria, e.g., MinPvalue, where `researcher->decisionStrategy` will prefer an outcome with the lowest
 p-value.
 */
enum class DecisionPreference {
    PreRegisteredOutcome,
    MinSigPvalue,
    MinPvalue,
    MaxSigEffect,
    MaxEffect,
    MinPvalueMaxEffect
};

const std::map<std::string, DecisionPreference>
stringToResearcherPreference = {
    {"Pre-registered Outcome", DecisionPreference::PreRegisteredOutcome},
    {"MinSigPvalue", DecisionPreference::MinSigPvalue},
    {"MinPvalue", DecisionPreference::MinPvalue},
    {"MaxSigEffect", DecisionPreference::MaxSigEffect},
    {"MaxEffect", DecisionPreference::MaxEffect},
    {"MinPvalueMaxEffect", DecisionPreference::MinPvalueMaxEffect}
};



/**
 \brief Abstract class for different decision strategies.
 
 */
class DecisionStrategy {
    
public:
    
    static DecisionStrategy* build(json &config);
    
    virtual ~DecisionStrategy() = 0;
    
    DecisionPreference selectionPref;     ///< Indicates researcher's selection preference on how he choose the outcome variable for submission.
    bool isStillHacking = true;         ///< If `true`, the Researcher will continue traversing through the hacknig methods, otherwise, he/she will stop the hacking and prepare the finalSubmission. It will be updated on each call of verdict(). Basically verdict() decides if the Researcher is happy with the submission record or not.
    
    int preRegGroup = 0;            ///< Indicates the pre-registered outcome in the case where the Researcher prefers the PreRegisteredOutcome
    Submission finalSubmission;     ///< This will set to the final submission recrod that the Researcher is satisfied about. At the same time, isStillHacking will set to `false`.
    
    virtual Submission selectOutcome(Experiment& experiment) = 0;
    
    virtual bool verdict(Experiment&, DecisionStage) = 0;
    
    // Sub-decisions
    virtual bool initDecision(Experiment &experiment) = 0;
    virtual bool intermediateDecision(Experiment &experiment) = 0;
    virtual bool afterhackDecision(Experiment &experiment) = 0;
    virtual bool finalDecision(Experiment &experiment) = 0;
    
    /**
     * \brief      Based on the DecisionPreference, it'll select the outcome
     * between all groups, `ng`. For instance, the MinPvalue deicison prefenrece will
     *
     * \param      experiment  
     *
     * \return     A copy of the selected outcome
     */
    Submission _select_Outcome(Experiment &experiment);
};

/**
 \brief Implementation of an impatient researcher. In this case, the Researcher will stop as soon as find a significant result and will not continue exploring other hacking methods in his arsenal.
 */
class ImpatientDecisionMaker : public DecisionStrategy {
    
public:
    
    std::vector<Submission> submissionsPool;
    std::vector<Experiment> experimentsPool;
    
    ImpatientDecisionMaker(DecisionPreference selection_pref){
        selectionPref = selection_pref;
    };
    
    Submission selectOutcome(Experiment &experiment) {
        return _select_Outcome(experiment);
    };
    
    bool isPublishable(const Submission &sub){
        return sub.isSig();
    }
    
    void clearPools(){
        experimentsPool.clear();
        submissionsPool.clear();
    }
    
    virtual bool verdict(Experiment &experiment, DecisionStage stage);
    
    virtual bool initDecision(Experiment &experiment);
    virtual bool intermediateDecision(Experiment &experiment);
    virtual bool afterhackDecision(Experiment &experiment);
    virtual bool finalDecision(Experiment &experiment);
    
};


class PatientDecisionMaker : public DecisionStrategy {

public:
    
    std::vector<Submission> submissionsPool;
    std::vector<Experiment> experimentsPool;
    
    PatientDecisionMaker(DecisionPreference selection_pref) {
        selectionPref = selection_pref;
    };
    
    Submission selectOutcome(Experiment &experiment){
        return _select_Outcome(experiment);
    };
    
    bool isPublishable(const Submission &sub){
        return sub.isSig();
    };
    
    virtual bool verdict(Experiment &experiment, DecisionStage stage);
    
    virtual bool initDecision(Experiment &experiment);
    virtual bool intermediateDecision(Experiment &experiment);
    virtual bool afterhackDecision(Experiment &experiment);
    virtual bool finalDecision(Experiment &experiment);
    
};



class HonestDecisionMaker : public DecisionStrategy {

public:

    
    HonestDecisionMaker(int pre_registered_group){
        preRegGroup = pre_registered_group;
    };

    Submission selectOutcome(Experiment& experiment) {
        return Submission(experiment, preRegGroup);
    };
    
    virtual bool verdict(Experiment &experiment, DecisionStage stage) {
        return true;
    };
    
    virtual bool initDecision(Experiment &experiment);
    virtual bool intermediateDecision(Experiment &experiment);
    virtual bool afterhackDecision(Experiment &experiment);
    virtual bool finalDecision(Experiment &experiment);
    
};

#endif //SAMPP_DECISIONSTRATEGY_H
