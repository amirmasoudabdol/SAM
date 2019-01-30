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

    // std::cout << submissionList.size();
    if (submissionList.size() == _max_pubs){
        _still_accepting = false;
    }
    // std::cout << s.pvalue << ": p\n";
}

void Journal::reject(Submission &s) {

}

void Journal::clear() {
    submissionList.clear();
    _still_accepting = true;
    // std::cout << submissionList.size() << "<<" << "max_pubs: " << _max_pubs;
}
