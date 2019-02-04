//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_HACKINGSTRATEGIES_H
#define SAMPP_HACKINGSTRATEGIES_H

#include "Experiment.h"
#include "SubmissionRecord.h"
#include <string>

enum HackingMethod {
    OUTCOME_SWITCHING,
    OUTLIER_REMOVAL
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
    virtual Submission perform(Experiment* experiment) = 0;
    
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
    Submission perform(Experiment* experiment);

private:
    std::string _method = "min pvalue";
};

class OptionalStopping : public HackingStrategy {
public:

    OptionalStopping(int n_new_obs, int n_trials) :
        _n_new_obs(n_new_obs), _n_trials(n_trials) {
        // selectedOutcome = 3;
    };

    // int selectedOutcome;
    // Submission hackedSubmission;
    Submission perform(Experiment* experiment);

private:
    int _n_new_obs, _n_trials;

};

// If I need more variations here, I think it's the best if I make different subclass
class OutlierRemoval : public HackingStrategy {
public:

    OutlierRemoval();

    // Submission hackedSubmission;
    Submission perform(Experiment* experiment);

};

class GroupPooling : public HackingStrategy {
public:
    
    GroupPooling(std::string scheme) : _scheme(scheme) {} ;
    
    // Submission hackedSubmission;
    Submission perform(Experiment* experiment);
    
private:
    std::string _scheme;
};

class QuestionableRounding : public HackingStrategy {
    
public:
    QuestionableRounding(int threshold) : _threshold(threshold) {};
    
private:
    int _threshold;
};

#endif //SAMPP_HACKINGSTRATEGIES_H
