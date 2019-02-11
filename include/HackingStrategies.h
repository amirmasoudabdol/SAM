//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_HACKINGSTRATEGIES_H
#define SAMPP_HACKINGSTRATEGIES_H

#include "Experiment.h"
#include "SubmissionRecord.h"
#include "DecisionStrategy.h"
#include <string>
#include <map>

enum HackingMethod {
    _OutcomeSwitching,
    _OptionalStopping,
    _OutlierRemoval
};




//Submission _create_submission_record(Experiment& experiment, int inx);

class HackingStrategy {

//    double defensibility;

public:

    // `perform()` makes a copy of the experiment, and perform the hacking.
    // This is usually a good idea, but its expensive if I have to do a lot of
    // them. Keep in mind that not all *HackingStrategies* need to modify the
    // *experiment*, so maybe later on, I can decide what kind of hacking I'm
    // performing, and decide if I need to make a copy or not.
    // Also, I still don't know how to implement the case where a researcher
    // wants to run several hacking over one experiment. I guess for this to be
    // done properly I need to do implement some sort of Template Pattern.
    virtual void perform(Experiment* experiment, DecisionStrategy* decisionStrategy) = 0;
    
    const int& getSelectedOutcome() const {
        return selectedOutcome;
    };
    void setSelectedOutcome(const int& _o_inx){
        selectedOutcome = _o_inx;
    }

private:
    int selectedOutcome;
    Submission hackedSubmission;
};

class OutcomeSwitching : public HackingStrategy {
public:

    OutcomeSwitching();
    OutcomeSwitching(std::string method) : _method(method) {

    };
    
    // Submission hackedSubmission;
    // TODO: I don't think it's even necessary for the `perform()` to return a Submission
    void perform(Experiment* experiment, DecisionStrategy* decisionStrategy);

private:
    std::string _method = "min pvalue";
};


/**
 \brief Declartion of OptionalStopping hacking strategy
 
 In order to apply the OptionalStopping, use the following format in the JSON
 config file where `n` specify number of new observations that is going to be
 added to the experiment in each trial, `m`.
 
 ```
 {
    "type": "OptionalStopping",
    "size": n,
    "attempts": m
 }
 ```
 
 */
class OptionalStopping : public HackingStrategy {
public:

    OptionalStopping(int n_new_obs, int n_trials) :
        _n_new_obs(n_new_obs), _n_trials(n_trials) {
        // selectedOutcome = 3;
    };

    // int selectedOutcome;
    // Submission hackedSubmission;
    void perform(Experiment* experiment, DecisionStrategy* decisionStrategy);

private:
    int _n_new_obs, _n_trials;

};

// If I need more variations here, I think it's the best if I make different subclass
/**
 \brief Declaration of SD-based Outlier Removal.
 
 `sd_multiplier` ...
 
 ```
     {
     "type": "SDOutlierRemoval",
     "sd_multiplier": d
     }
 
 ```
 */
class SDOutlierRemoval : public HackingStrategy {
public:

    SDOutlierRemoval(double sd_multiplier) : _sd_multiplier(sd_multiplier) {};

    // Submission hackedSubmission;
    void perform(Experiment* experiment, DecisionStrategy* decisionStrategy);
    
private:
    double _sd_multiplier;      ///< Specifies the multiplier threshold for removing the outliers

};

class GroupPooling : public HackingStrategy {
public:
    
    GroupPooling(std::string scheme) : _scheme(scheme) {} ;
    
    // Submission hackedSubmission;
    void perform(Experiment* experiment, DecisionStrategy* decisionStrategy);
    
private:
    std::string _scheme;
};

//class QuestionableRounding : public HackingStrategy {
//
//public:
//    QuestionableRounding(int threshold) : _threshold(threshold) {};
//
//private:
//    int _threshold;
//};

#endif //SAMPP_HACKINGSTRATEGIES_H
