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

    // `perform()` makes a copy of the experiment, and perform the hacking.
    // This is usually a good idea, but its expensive if I have to do a lot of
    // them. Keep in mind that not all *HackingStrategies* need to modify the
    // *experiment*, so maybe later on, I can decide what kind of hacking I'm
    // performing, and decide if I need to make a copy or not.
    // Also, I still don't know how to implement the case where a researcher
    // wants to run several hacking over one experiment. I guess for this to be
    // done properly I need to do implement some sort of Template Pattern.
    virtual void perform(Experiment* experiment, DecisionStrategy* decisionStrategy) = 0;
};


/**
 \brief Declartion of OptionalStopping hacking strategy
 
 In order to apply the OptionalStopping, use the following format in the JSON
 config file where `n` specify number of new observations that is going to be
 added to the experiment in each trial, `m`.
 
 ```
 {
    "type": "Optional Stopping",
    "size": n,
    "attempts": m
 }
 ```
 
 */
class OptionalStopping : public HackingStrategy {
public:

    OptionalStopping(std::string mode, std::string level, int num, int n_attempts, int max_attempts) :
        _mode(mode),
        _level(level),
        _num(num),
        _n_attempts(n_attempts),
        _max_attempts(max_attempts)
    {
        if (mode == "Extreme"){
            _num = 1;
            _n_attempts = _max_attempts;
        }
    };

    void perform(Experiment *experiment, DecisionStrategy *decisionStrategy);

private:
    std::string _mode = "Recursive";
    std::string _level = "dv";
    int _num;
    int _n_attempts;
    int _max_attempts = 10;
    
    void addObservations(Experiment *experiment, const int &n);

};

/**
 \brief Declaration of Outlier Removal hacking method based on items' distance from their
 sample mean.
 
 `sd_multiplier` ...
 
 ```
     {
     "type": "SD Outlier Removal",
     "sd_multiplier": d
     }
 
 ```
 */
class SDOutlierRemoval : public HackingStrategy {
public:

    SDOutlierRemoval(std::string mode, std::string level, std::string order, int num, int n_attempts, int max_attempts, int min_observations, std::vector<double> multipliers) :
        _mode(mode),
        _level(level),
        _order(order),
        _num(num),
        _n_attempts(n_attempts),
        _max_attempts(max_attempts),
        _min_observations(min_observations),
        _multipliers(multipliers)
    {
        
        if (_mode == "Extreme") {
            _num = 1;
            _n_attempts = _MAX_ITERS;
            _max_attempts = _MAX_ITERS;
        }else if (_mode == "Recursive"){
            _n_attempts = _MAX_ITERS;
            _max_attempts = _MAX_ITERS;
        }
        
    };
    

    // Submission hackedSubmission;
    void perform(Experiment* experiment, DecisionStrategy* decisionStrategy);
    
private:
    std::string _mode = "Recursive Attempts";
    std::string _level = "dv";
    std::string _order = "max first";
    int _num;
    int _n_attempts;
    int _max_attempts;
    int _min_observations;
    std::vector<double> _multipliers;
    
    int _MAX_ITERS = 100;
    
    int removeOutliers(Experiment *experiment, const int &n, const int &d);
    
//    double _sd_multiplier;      ///< Specifies the multiplier threshold for removing the outliers

};

class GroupPooling : public HackingStrategy {
public:
    
    GroupPooling() {};
    
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
