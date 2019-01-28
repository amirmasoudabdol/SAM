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

    void setSelectionStrategy(SelectionStrategy* s);

    bool review(Submission& s);
    void accept(Submission s);
    void reject(Submission& s);

};

#endif //SAMPP_JOURNAL_H
