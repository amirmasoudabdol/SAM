//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <Journal.h>
#include <iostream>
#include <iomanip>

//#include "main.h"

extern bool VERBOSE;


Journal::Journal(json& config){
    _max_pubs = config["--max-pubs"];
    _pub_bias = config["--pub-bias"];
    _alpha = config["--alpha"];
    
    // This can even initialize and set the SelectionStrategy
    
    this->selectionStrategy = SelectionStrategy::build(config);
}

void Journal::setSelectionStrategy(SelectionStrategy *s) {
    selectionStrategy = s;
}

bool Journal::review(const Submission &s) {

    bool decision = this->selectionStrategy->review(s);

    if (decision){
        accept(s);
    }else{
        reject(s);
    }
    return decision;
}

void Journal::accept(const Submission s) {

    publicationList.push_back(s);

    if (publicationList.size() == _max_pubs){
        _still_accepting = false;
    }
    
}

void Journal::reject(const Submission &s) {

}

void Journal::clear() {
    publicationList.clear();
    _still_accepting = true;
}

void Journal::saveSubmissions(int simid, std::ofstream& writer) {
    
    int i = 0;
    for (auto& p : publicationList) {
        p.simid = simid;
        p.pubid = i++;
        p.pubbias = _pub_bias;
        
        // TODO: I need a global DEBUG variable
         if (VERBOSE){
            std::cout << std::setprecision(8);
            std::cout << p << "\n";
         }
        
        writer << p << "\n";
    }
}

