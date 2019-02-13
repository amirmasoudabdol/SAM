//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_SELECTIONSTRATEGIES_H
#define SAMPP_SELECTIONSTRATEGIES_H

#include "SubmissionRecord.h"
#include "RandomNumberGenerator.h"

enum SelectionType {
    signigicantSelection,
};

/**
 \brief Abstract class for Journal's selection strategies.
 
 A Journal will decide if a Submission is going to be accepted or rejected. This
 decision can be made based on different criteria or formula. A SelectionStrategy
 provides an interface for implementing different selection strategies.
 */
class SelectionStrategy{

public:

    virtual bool review(Submission& s) = 0 ;
};

/**
 \brief Significant-based Selection Strategy
 
 Significant-based selection strategy accepts a publication if the given _p_-value
 is significant. Certain degree of _Publication Bias_, can be specified. In this case,
 a Submission has a chance of being published even if the statistics is not significant.
 Moreover, the SignificantSelection can be tailored toward either positive or negative
 effect. In this case, the Journal will only accept Submissions with larger or smaller effects.
 */
class SignigicantSelection : public SelectionStrategy {

public:
    SignigicantSelection(double alpha, double pub_bias, int side, int seed):
        _alpha(alpha),  _pub_bias(pub_bias), _side(side), _seed(seed) {
        _rngEngine = new RandomNumberGenerator(_seed, false);
    };

    ~SignigicantSelection(){};

    bool review(Submission& s);

private:
    double _alpha;
    double _pub_bias;
    int _side;
    int _seed;
    

    RandomNumberGenerator* _rngEngine;
};

#endif //SAMPP_SELECTIONSTRATEGIES_H
