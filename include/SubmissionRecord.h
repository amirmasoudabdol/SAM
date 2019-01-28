//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_SUBMISSIONRECORD_H
#define SAMPP_SUBMISSIONRECORD_H

#include <string>
#include <iostream>

class Submission {

public:
    int simid = 0;
    int pubid = 0;
    std::string method;
    double effect;
    double stat;
    double pvalue;
    bool sig = false;
    short side = 1;

};

//std::ostream& operator<<(std::ostream& os, const Submission& s);
//
//
//std::ostream& operator<<(std::ostream& os, const Submission& s){
////	return os << s.simid << ", " <<
////			  s.pubid << ", " <<
////			  s.method << ", " <<
////			  s.effect << ", " <<
////			  s.stat << ", " <<
////			  s.pvalue << ", " <<
////			  s.sig << ", " <<
////			  s.side;
//	return os;
//}



#endif //SAMPP_SUBMISSIONRECORD_H
