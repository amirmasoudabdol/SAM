//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <Journal.h>
#include <iostream>
#include <iomanip>
#include <stdexcept>

//#include "main.h"

extern bool VERBOSE;


Journal::Journal(json& config){
    
    max_pubs = config["max-pubs"];
    
    // Setting up the SelectionStrategy
    this->selectionStrategy = SelectionStrategy::build(config["selection-strategy"]);
}

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
    
    publicationList.push_back(s);
    
    if (publicationList.size() == max_pubs){
        still_accepting = false;
    }
    
}

void Journal::reject(Submission &s) {
    
}

void Journal::clear() {
    publicationList.clear();
    still_accepting = true;
}

void Journal::saveSubmissions(int simid, std::ofstream& writer) {
    
    int i = 0;
    for (auto& p : publicationList) {
        p.simid = simid;
        p.pubid = i++;
//        if (selectionStrategy->id == SelectionType::SignificantSelection){
//            p.pubbias = selection;
//        }
        
        if (VERBOSE){
            std::cout << std::setprecision(8);
            std::cout << p << "\n";
        }
        
        writer << p << "\n";
    }
}

