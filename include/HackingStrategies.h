//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_HACKINGSTRATEGIES_H
#define SAMPP_HACKINGSTRATEGIES_H

#include "Experiment.h"
#include "SubmissionRecord.h"
#include <string>

class HackingStrategy {
public:
//    HackingStrategy();

    virtual Submission perform() = 0;
    virtual Submission performOnCopy(Experiment expr) = 0;
    // virtual void setExperiment() = 0;
    
    double defensibility;
};

class OutcomeSwitching : public HackingStrategy {
public:
    Experiment* experiment;


    OutcomeSwitching();
    OutcomeSwitching(std::string method) : _method(method) {

    };

    OutcomeSwitching(Experiment* e, std::string method) :
        experiment(e), _method(method) {
        // TODO: I might need a copy constructor in the Experiment to be able to copy the pointers.

    }

    void setExperiment(Experiment& e);
    
    Submission hackedSubmission;
    Submission perform();
    Submission performOnCopy(Experiment expr);

private:
    std::string _method = "min pvalue";
//    long _selected_outcome_inx = 0;
    Submission _create_submission_record(int inx);

};

class OptionalStopping : public HackingStrategy {
public:
    Experiment* experiment;


    OptionalStopping(int max_new_obs) : _max_new_obs(max_new_obs) {};

    // OptionalStopping(Experiment* e, int max_new_obs) :
    //     experiment(e), _max_new_obs(max_new_obs){

    // };

    Submission hackedSubmission;
    Submission perform();

private:
    int _max_new_obs;

};

// If I need more variations here, I think it's the best if I make different subclass
class OutlierRemoval : public HackingStrategy {
public:
    Experiment* experiment;

    OutlierRemoval();

    Submission hackedSubmission;
    Submission perform();

};

class GroupPooling : public HackingStrategy {
public:
    Experiment* experiment;
    
    GroupPooling(std::string scheme) : _scheme(scheme) {} ;
    
    Submission perform();
    Submission hackedSubmission;
    
private:
    std::string _scheme;
};

class QuestionableRounding : public HackingStrategy {
    int _threshold;
    
public:
    QuestionableRounding(int threshold) : _threshold(threshold) {};
};

#endif //SAMPP_HACKINGSTRATEGIES_H
