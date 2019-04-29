//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <Journal.h>
#include <iostream>
#include <iomanip>
#include <stdexcept>

#include "MetaAnalysis.h"

using namespace sam;

extern bool VERBOSE;


Journal::Journal(json& config){
    
    max_pubs = config["max-pubs"];
    
    // Setting up the SelectionStrategy
    this->selection_strategy = SelectionStrategy::build(config["selection-strategy"]);
}

void Journal::setSelectionStrategy(SelectionStrategy *s) {
    selection_strategy = s;
}

bool Journal::review(Submission &s) {
    
    bool decision = this->selection_strategy->review(s);
    
    if (decision){
        accept(s);
    }else{
        reject(s);
    }
    return decision;
}

void Journal::accept(Submission s) {
    
    publications_list.push_back(s);
    
    if (publications_list.size() == max_pubs){
        still_accepting = false;
    }
    
}

void Journal::reject(Submission &s) {
    
}

void Journal::clear() {
    publications_list.clear();
    still_accepting = true;
}

void Journal::saveSubmissions(int simid, std::ofstream& writer) {
    
    int i = 0;
    for (auto& p : publications_list) {
        p.simid = simid;
        p.pubid = i++;
        
        if (VERBOSE){
            std::cout << std::setprecision(8);
            std::cout << p << "\n";
        }
        
        writer << p << "\n";
    }
}


void Journal::testMeta() {
    FixedEffectEstimator fes;
    
    cerr << fes.estimate(publications_list);
}
