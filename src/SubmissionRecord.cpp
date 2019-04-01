//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include "SubmissionRecord.h"

std::ostream& operator<<(std::ostream& os, const Submission& s){
	os <<
            s.simid << "," <<
            s.pubid << "," <<
//            s.method << "," <<
            s.tnobs << "," <<
            s.tyi << "," <<
            s.tvi << "," <<
//            s.tcov << "," <<
            s.inx << "," <<
            s.nobs << "," <<
            s.yi << "," <<
            s.sei << "," <<
            s.statistic << "," <<
            s.pvalue << ",";
//            s.effect << "," <<
            for (auto &e : s.effects){
                os << e << ",";
            };
            os << s.side;

//            os << s.isHacked;
    
    return os;
}


std::string Submission::header(const json &effectslist) {
    std::string header{"simid,pid,tnobs,tyi,tvi,inx,nobs,yi,sei,statistic,pvalue,"};
    
    for (auto &estimator : effectslist){
        header += estimator;
        header += ",";
    }
    
    header += "side";
    
    return header;
}
