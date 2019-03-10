//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <Journal.h>
#include <iostream>
#include <iomanip>


Journal::Journal(json& config){
    _max_pubs = config["--max-pubs"];
    _pub_bias = config["--pub-bias"];
    _alpha = config["--alpha"];
    
    // This can even initialize and set the SelectionStrategy
    
    this->selectionStrategy = SelectionStrategy::buildSelectionStrategy(config);
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

    // Updating the accepted submission by some of the Journal specific paramters
    // FIXME: The conflict between the decison strategy and Journal shows itself here as well.
    // Should I update them here or during the decision making. 
    // See also: [#50](https://github.com/amirmasoudabdol/SAMpp/issues/50)

    submissionList.push_back(s);

    if (submissionList.size() == _max_pubs){
        _still_accepting = false;
    }
    
}

void Journal::reject(Submission &s) {

}

void Journal::clear() {
    submissionList.clear();
    _still_accepting = true;
}

void Journal::saveSubmissions(int simid, std::ofstream& writer) {
    
    int i = 0;
    for (auto& p : submissionList) {
        p.simid = simid;
        p.pubid = i++;
        p.pubbias = _pub_bias;
        
        // TODO: I need a global DEBUG variable
        if (true)
            std::cout << std::setprecision(8);
            std::cout << p << "\n";
        
        writer << p << "\n";
    }
}

