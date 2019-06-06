//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <Journal.h>
#include <iostream>
#include <iomanip>
#include <stdexcept>

#include "MetaAnalysis.h"
#include "SelectionStrategy.h"

using namespace sam;

Journal::Journal(json& journal_config){
    
    max_pubs = journal_config["max_pubs"];
    
    // Setting up the SelectionStrategy
    this->selection_strategy = SelectionStrategy::build(journal_config["selection_strategy"]);
}

//Journal::Journal(JournalParameters &jp,
//                 SelectionStrategy::SelectionStrategyParameters &ssp) {
//    params = jp;
//    max_pubs = jp.max_pubs;
//    this->selection_strategy = SelectionStrategy::build(ssp);
//}

bool Journal::review(Submission s) {

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

void Journal::testMeta() {
    FixedEffectEstimator fes;
    
    cerr << fes.estimate(publications_list);
}
