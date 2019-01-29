//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <Journal.h>
#include <iostream>

void Journal::setSelectionStrategy(SelectionStrategy *s) {
    selectionStrategy = s;
}

bool Journal::review(Submission &s) {

    bool decision = this->selectionStrategy->review(s);

    if (decision){
        accept(s);
    }else{
        reject(s);
    }
    return decision;
}

void Journal::accept(Submission s) {
    submissionList.push_back(s);
    // std::cout << s.pvalue << ": p\n";
}

void Journal::reject(Submission &s) {

}
