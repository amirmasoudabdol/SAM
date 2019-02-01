//
// Created by Amir Masoud Abdol on 2019-02-01.
//

#ifndef SAMPP_DECISIONSTRATEGY_H
#define SAMPP_DECISIONSTRATEGY_H

#include <vector>

#include "SubmissionRecord.h"
#include "Experiment.h"

class DecisionStrategy {

public:
    virtual Submission selectOutcome(Experiment& experiment) = 0;
    
    // These two are quite similar, and they are being used in the prepareForSubmission
    // routine where the researcher makes the final decision, in this way, based on how
    // aggressive his decision making is, he can choose between travesing all groups — all
    // the way back, through all hacking — or travese only between the submissions that he
    // has previously selected.
    // Check *Researcher* for more info on more complex setup
    virtual Submission selectBetweenSubmission(std::vector<Submission>& submissions) = 0;
    virtual Submission selectBetweenExperiments(std::vector<Experiment>& experiments) = 0;
    
};

class ReportPreregisteredGroup : public DecisionStrategy {

    int _pre_registered_group = 0;

public:
    ReportPreregisteredGroup(int pre_registered_group) :
        _pre_registered_group(pre_registered_group) {};

    Submission selectOutcome(Experiment& experiment) {
        return Submission(experiment, _pre_registered_group);
    };

    Submission selectBetweenSubmission(std::vector<Submission>& submissions){
        
        return submissions.back(); // FIXME: For now this is alright!
    };
    Submission selectBetweenExperiments(std::vector<Experiment>& experiments){
        return Submission();
    };
    
};

#endif //SAMPP_DECISIONSTRATEGY_H
