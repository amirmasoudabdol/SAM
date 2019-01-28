//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_SUBMISSIONRECORD_H
#define SAMPP_SUBMISSIONRECORD_H

#include <string>
#include <iostream>

struct Submission{
    int simid = 0;
    int pubid = 0;
    std::string method;
    double stats;
    double pvalue;
    bool sig;
    bool side;
};

//std::ostream& operator<<(std::ostream& os, const Submission& sub){
//    return os << sub.simid << ", " <<
//                 sub.pubid << ", " <<
//                 sub.method << ", " <<
//                 sub.stats << ", " <<
//                 sub.pvalue << ", " <<
//                 sub.sig << ", " <<
//                 sub.side << ", ";
//}

#endif //SAMPP_SUBMISSIONRECORD_H
