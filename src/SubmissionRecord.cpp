//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include "SubmissionRecord.h"

std::ostream& operator<<(std::ostream& os, const Submission& s){
	return os <<
            s.simid << ", " <<
            s.pubid << ", " <<
//            s.method << ", " <<
            s.tnobs << ", " <<
            s.tyi << ", " <<
            s.tvi << ", " <<
            s.tcov << ", " <<
            s.inx << ", " <<
            s.nobs << ", " <<
            s.yi << ", " <<
            s.sei << ", " <<
            s.statistic << ", " <<
            s.pvalue << ", " <<
            s.effect << ", " <<
            s.side;
}
