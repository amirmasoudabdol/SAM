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

// This is a reference for their id as well
enum class HackingMethod : unsigned int {
    OptionalStopping = 1,       // 1
    SDOutlierRemoval,       // 2
    GroupPooling            // 3
};

const std::map<std::string, HackingMethod>
stringToHackingMethod = {
    {"OptionalStopping", HackingMethod::OptionalStopping},
    {"SDOutlierRemoval", HackingMethod::SDOutlierRemoval},
    {"GroupPooling", HackingMethod::GroupPooling}         
};

/**
 \brief Abstract class for hacking strategies.
 
 Each HackingStrategy should provide a `perform()` method. The `perform()` method
 will take over a pointer to an Experiment and apply the implemented hacking on it. 
 Researcher decides if this is a pointer to a *fresh* copy of the Experiment or a pointer
 to a previously "hacked" Experiment.

 */
class HackingStrategy {

protected:
    //! Defensibility of the method
    //! This is a based on the survey results where researchers have been
    //! asked to rate the defensibility of different QRPs.
    
    

public:
    double defensibility;
    HackingMethod hid;
    
    /**
     * \brief      Factory method for building a HackingStrategy
     *
     * \param      config  A reference to an item of the `json['--hacking-strategy']`.
     *                     Researcher::Builder is responsible for passing this object
     *                     correctly. 
     *
     * \return     A new HackingStrategy
     */
    static HackingStrategy* build(json& config);
    

    /**
     * \brief      Pure deconstuctor of the Base calss. This is important
     * for proper deconstruction of Derived classes.
     */
    virtual ~HackingStrategy() = 0;

    /**
     * \brief      Applies the hacking method on the Experiment.
     *
     * \param      experiment        A pointer to an Experiment.
     *                               
     * \param      decisionStrategy  A pointer to Researcher's DecisionStrategy.
     *                               The HackingStrategy decides with what flag it
     *                               is going to use the DecisionStrategy.
     */
    virtual void perform(Experiment* experiment, DecisionStrategy* decisionStrategy) = 0;
};


/**
 \brief Declartion of OptionalStopping hacking strategy
 */
class OptionalStopping : public HackingStrategy {
public:

    OptionalStopping(std::string level, int num, int n_attempts, int max_attempts) :
        _level(level),
        _num(num),
        _n_attempts(n_attempts),
        _max_attempts(max_attempts)
    {
        hid = HackingMethod::OptionalStopping;
    };

    void perform(Experiment *experiment, DecisionStrategy *decisionStrategy);
    
    
    
private:
    
    std::string _level = "dv";
    int _num = 3;
    int _n_attempts = 3;
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
        hid = HackingMethod::SDOutlierRemoval;
    };
    

    // Submission hackedSubmission;
    void perform(Experiment* experiment, DecisionStrategy* decisionStrategy);
    
    
    
private:
    
    std::string _level = "dv";
    std::string _order = "max first";
    int _num = 3;
    int _n_attempts = 1;
    int _max_attempts = 10;
    int _min_observations = 15;
    std::vector<double> _multipliers = {3, 2, 1};
    
    int _MAX_ITERS = 100;
    
    int removeOutliers(Experiment *experiment, const int &n, const int &d);
    
};

class GroupPooling : public HackingStrategy {
        
public:
    
    GroupPooling(std::vector<int> nums) : _nums(nums)
    {
        hid = HackingMethod::GroupPooling;
    };
    
    // Submission hackedSubmission;
    void perform(Experiment* experiment, DecisionStrategy* decisionStrategy);
    
private:

    std::vector<int> _nums = {2};
    
    void pool(Experiment* experiment, int r);
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
