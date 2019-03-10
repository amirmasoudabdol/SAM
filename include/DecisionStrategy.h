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
    {"Min Sig P-value", DecisionPreference::MinSigPvalue},
    {"Min P-value", DecisionPreference::MinPvalue},
    {"Max Sig Effect", DecisionPreference::MaxSigEffect},
    {"Max Effect", DecisionPreference::MaxEffect},
    {"Min P-value, Max Effect", DecisionPreference::MinPvalueMaxEffect}
};



/**
 \brief Abstract class for different decision strategies.
 
 */
class DecisionStrategy {
    
public:
    
    static DecisionStrategy* buildDecisionStrategy(json& config);
    
    DecisionPreference selectionPref;     ///< Indicates researcher's selection preference on how he choose the outcome variable for submission.
    bool isStillHacking = true;         ///< If `true`, the Researcher will continue traversing through the hacknig methods, otherwise, he/she will stop the hacking and prepare the finalSubmission. It will be updated on each call of verdict(). Basically verdict() decides if the Researcher is happy with the submission record or not.
    
    int preRegGroup = 0;            ///< Indicates the pre-registered outcome in the case where the Researcher prefers the PreRegisteredOutcome
    Submission finalSubmission;     ///< This will set to the final submission recrod that the Researcher is satisfied about. At the same time, isStillHacking will set to `false`.
    
    // TODO: I don't use this at the moment, I basically thought I need it for cases like
    // MinSigPvalue
//    double alpha = 0.05;
    
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
    
    virtual bool verdict(Experiment&, DecisionStage) = 0;
    
    //    virtual void finalDecision();
    
    Submission _select_Outcome(Experiment&);
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
    
    void verdict(std::vector<Submission>& submissions, std::vector<Experiment>& experiments) {
        
        finalSubmission = submissions.back();
        
        // TODO: Move the significance test to the Submission
        if (submissions.back().isSig()) {
            isStillHacking = false;
        }else{
            isStillHacking = true;
        }
    }
    
    void clearPools(){
        experimentsPool.clear();
        submissionsPool.clear();
    }
    
    bool verdict(Experiment &experiment, DecisionStage stage) {
        switch(stage){
            case DecisionStage::Initial:
                {
                    Submission sub = selectOutcome(experiment);
        
                    // Preparing pools anyway
                    experimentsPool.push_back(experiment);
                    submissionsPool.push_back(sub);
        
                    if (isPublishable(sub)){
                        return true;
                    }else{
                        return false;
                    }
                }
                break;
            case DecisionStage::WhileHacking:
            {
                bool publishable = isPublishable(selectOutcome(experiment));
                    isStillHacking = publishable;
                return publishable;
            }
                break;
            case DecisionStage::DoneHacking:
                {
                    Submission sub = selectOutcome(experiment);
                    if (isPublishable(sub)){
                        experimentsPool.push_back(experiment);
                        submissionsPool.push_back(sub);
                        
                        return true;
                    }else{
                        isStillHacking = true;
                        return isStillHacking;
                    }
                }
                break;
            case DecisionStage::Final:
                // TODO: This can be implemented differenly if necessary
            {
                finalSubmission = submissionsPool.back();
                experimentsPool.clear();
                submissionsPool.clear();
                return true;
                
            }
                break;
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
    
    void verdict(std::vector<Submission>& submissions, std::vector<Experiment>& experiments) {
        isStillHacking = true;
    }
    
    bool verdict(Experiment &experiment, DecisionStage stage) {
        return true;
    };

//    Submission selectBetweenSubmission(std::vector<Submission>& submissions){
//
//        return submissions.back(); // FIXME: For now this is alright!
//    };
//    Submission selectBetweenExperiments(std::vector<Experiment>& experiments){
//        return Submission();
//    };
    
};

#endif //SAMPP_DECISIONSTRATEGY_H
