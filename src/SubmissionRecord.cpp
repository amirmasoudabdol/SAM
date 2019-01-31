//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include "SubmissionRecord.h"

std::ostream& operator<<(std::ostream& os, const Submission& s){
	return os <<
//            s.simid << ", " <<
//            s.pubid << ", " <<
//            s.method << ", " <<
            s.effect << ", " <<
            s.stat << ", " <<
            s.pvalue << ", " <<
//            s.sig << ", " <<
            s.side;
}
