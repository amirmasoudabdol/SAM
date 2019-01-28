//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_JOURNAL_H
#define SAMPP_JOURNAL_H

#include <vector>

#include "SelectionStrategies.h"
#include "SubmissionRecord.h"

class Journal{
public:

    std::vector<Submission> submissionList;

    SelectionStrategy* selectionStrategy;

    Journal(double max_pubs, double pub_bias, double alpha) :
            _max_pubs(max_pubs), _pub_bias(pub_bias), _alpha(alpha){
        _still_accepting = true;
    };

    void setSelectionStrategy(SelectionStrategy* s);

    bool review(Submission& s);
    void accept(Submission s);
    void reject(Submission& s);

    bool isStillAccepting(){
        return _still_accepting;
    }

private:
    double _max_pubs;
    double _pub_bias;
    double _alpha;

    bool _still_accepting;

};

#endif //SAMPP_JOURNAL_H
