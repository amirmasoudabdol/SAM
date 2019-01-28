//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_SELECTIONSTRATEGIES_H
#define SAMPP_SELECTIONSTRATEGIES_H

#include "SubmissionRecord.h"
#include "RandomNumberGenerator.h"

class SelectionStrategy{

public:

    virtual bool review(Submission& s) = 0 ;
};

class SignigicantSelection : public SelectionStrategy {

public:
    SignigicantSelection(double alpha, double pub_bias):
        _alpha(alpha),  _pub_bias(pub_bias) {
        // TODO: initialize without seed, basically a random seed
        _rngEngine = new RandomNumberGenerator(42, false);
    };

    ~SignigicantSelection(){};

    bool review(Submission& s);

private:
    double _pub_bias;
    double _alpha;
    RandomNumberGenerator* _rngEngine;
};

#endif //SAMPP_SELECTIONSTRATEGIES_H
