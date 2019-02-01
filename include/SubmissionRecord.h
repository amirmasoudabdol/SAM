//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_SUBMISSIONRECORD_H
#define SAMPP_SUBMISSIONRECORD_H

#include <string>
#include <iostream>
#include <Experiment.h>

class Submission {

public:
    int simid = 0;
    int pubid = 0;
//    std::string method;
    double effect;
    double statistic;
    double pvalue;
//    bool sig = false;
//    short side = 1;
//    bool isHacked = false;
//    std::string hacking_method = "";

    Submission() = default;
    
    Submission(Experiment& e, int index){
        effect = e.effects[index];
        statistic = e.statistics[index];
        pvalue = e.pvalues[index];
    };
    
    ~Submission() = default;

    friend std::ostream& operator<<(std::ostream& os, const Submission& s);

};

#endif //SAMPP_SUBMISSIONRECORD_H
