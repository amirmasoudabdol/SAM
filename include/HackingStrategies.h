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

    Submission hackedSubmission;
    Submission perform();

private:
    std::string _method = "Min PValue";
    long _selected_outcome_inx = 0;
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

#endif //SAMPP_HACKINGSTRATEGIES_H
