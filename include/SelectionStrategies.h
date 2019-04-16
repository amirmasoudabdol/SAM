//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_SELECTIONSTRATEGIES_H
#define SAMPP_SELECTIONSTRATEGIES_H

#include "SubmissionRecord.h"
#include "RandomNumberGenerator.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

enum class SelectionType {
    SignigicantSelection,
};

/**
 \brief Abstract class for Journal's selection strategies.
 
 A Journal will decide if a Submission is going to be accepted or rejected. This
 decision can be made based on different criteria or formula. A SelectionStrategy
 provides an interface for implementing different selection strategies.
 */
class SelectionStrategy{
    
protected:
    RandomNumberGenerator* mainRngStream;

public:
    
    /**
     * \brief      Factory method for building a SelectionStrategy
     *
     * \param      config  A reference to `json['Journal Parameters'].
     * Usually Researcher::Builder is responsible for passing the object
     * correctly.
     *
     * \return     A new SelectionStrategy
     */
    static SelectionStrategy* build(json &config);
    
    /**
     * \brief      Pure deconstructor of the base class
     */
    virtual ~SelectionStrategy() = 0;

    /**
     * \brief      Review the Submission and decides if it's
     * going to be accepted or rejected. When deriving from 
     * SelectionStrategy, `review` is the main interface and 
     * `Journal` relies on its output
     *
     * \param[in]  s     A reference to a Submission
     *
     * \return     A boolean indicating whether the Submission
     * should be accepted or not.
     */
    virtual bool review(const Submission& s) = 0 ;
};

/**
 \brief Significant-based Selection Strategy
 
 Significant-based selection strategy accepts a publication if the given *p*-value
 is significant. Certain degree of *publication bias*, can be specified. In this case,
 a Submission has a chance of being published even if the statistics is not significant.
 Moreover, the SignificantSelection can be tailored toward either positive or negative
 effect. In this case, the Journal will only accept Submissions with larger or smaller effects.
 */
class SignigicantSelection : public SelectionStrategy {

public:
    SignigicantSelection(double alpha, double pub_bias, int side, int seed):
        alpha(alpha),  pub_bias(pub_bias), side(side), seed(seed) {
        
        mainRngStream = new RandomNumberGenerator(seed);
    };

    ~SignigicantSelection(){};

    bool review(const Submission& s);

    //! The \alpha at which the _selection strategy_ decides the significance
    //! of a publication
    double alpha;
    
    //! Publication bias rate
    double pub_bias;
    
    //! Indicates the _selection stratgy_'s preference toward positive, `1`,
    //! or negative, `-1` effect. If `0`, Journal doesn't have any preferences.
    int side;
    
    int seed;
};

class RandomSelection : public SelectionStrategy {
public:
    RandomSelection(int seed) : seed(seed) {
        mainRngStream = new RandomNumberGenerator(seed);
    }
    
    bool review(const Submission& s);
    
    int seed;
};

#endif //SAMPP_SELECTIONSTRATEGIES_H
