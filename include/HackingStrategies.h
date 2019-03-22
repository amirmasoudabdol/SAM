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

#include "nlohmann/json.hpp"

using json = nlohmann::json;

enum class HackingMethod {
    OptionalStopping,
    OutlierRemoval,
    GroupPooling
};
const std::map<std::string, HackingMethod>
stringToHackingMethod = {
    {"Optional Stopping", HackingMethod::OptionalStopping},
    {"SD Outlier Removal", HackingMethod::OutlierRemoval},
    {"Group Pooling", HackingMethod::GroupPooling},
};

/**
 \brief Abstract class for Hacking Strategies
 
 Each _Hacking Strategy_ should provide a `perform()` method. The `perform()` method
 will take over a **pointer** copy of an Experiment and apply the implemented hacking on it. Changes
 will be applied to the copy and will be return to the Researcher.
 
 \note Researcher decides if it's going to pass a copy or a reference to an existing Experiment. This is important since it allows the researcher to either stack
    different hacking over each others or pass a new copy — usually the original experiment —
 to the experiment.
 */
class HackingStrategy {

//    double defensibility;

public:
    
    static HackingStrategy* buildHackingMethod(json& config);
    
    virtual ~HackingStrategy() = 0;

    /**
     * \brief      Applies the hacking method on the Experiment.
     *
     * \param      experiment        A pointer to an Experiment.
     *                               Researcher decides if this is a pointer to
     *                               a *fresh* copy of the Experiment or a pointer 
     *                               to a previously "hacked" Experiment.
     * \param      decisionStrategy  The decision strategy
     */
    virtual void perform(Experiment* experiment, DecisionStrategy* decisionStrategy) = 0;
};


/**
 \brief Declartion of OptionalStopping hacking strategy
 
 In order to apply the OptionalStopping, use the following format in the JSON
 config file where `n` specify number of new observations that is going to be
 added to the experiment in each trial, `m`.
 
 */
class OptionalStopping : public HackingStrategy {
public:

    OptionalStopping(std::string level, int num, int n_attempts, int max_attempts) :
        _level(level),
        _num(num),
        _n_attempts(n_attempts),
        _max_attempts(max_attempts)
    {

    };

    void perform(Experiment *experiment, DecisionStrategy *decisionStrategy);

private:
//    std::string _mode = "Recursive";
    std::string _level = "dv";
    int _num;
    int _n_attempts;
    int _max_attempts = 10;
    
    void addObservations(Experiment *experiment, const int &n);

};

/**
 \brief Declaration of Outlier Removal hacking method based on items' distance from their
 sample mean.
 
 */
class SDOutlierRemoval : public HackingStrategy {
public:

    SDOutlierRemoval(std::string level, std::string order, int num, int n_attempts, int max_attempts, int min_observations, std::vector<double> multipliers) :
        _level(level),
        _order(order),
        _num(num),
        _n_attempts(n_attempts),
        _max_attempts(max_attempts),
        _min_observations(min_observations),
        _multipliers(multipliers)
    {
        
//        if (_mode == "Extreme") {
//            _num = 1;
//            _n_attempts = _MAX_ITERS;
//            _max_attempts = _MAX_ITERS;
//        }else if (_mode == "Recursive"){
//            _n_attempts = _MAX_ITERS;
//            _max_attempts = _MAX_ITERS;
//        }
        
    };
    

    // Submission hackedSubmission;
    void perform(Experiment* experiment, DecisionStrategy* decisionStrategy);
    
private:
//    std::string _mode = "Recursive Attempts";
    std::string _level = "dv";
    std::string _order = "max first";
    int _num;
    int _n_attempts;
    int _max_attempts;
    int _min_observations;
    std::vector<double> _multipliers;
    
    int _MAX_ITERS = 100;
    
    int removeOutliers(Experiment *experiment, const int &n, const int &d);
    
};

class GroupPooling : public HackingStrategy {
    
private:
    int _num = 2;
    
public:
    
    GroupPooling(int num) : _num(num) {};
    
    // Submission hackedSubmission;
    void perform(Experiment* experiment, DecisionStrategy* decisionStrategy);
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
