//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include "SubmissionRecord.h"

std::ostream& operator<<(std::ostream& os, const Submission& s){
	os <<
    s.simid << "," <<
    s.pubid << "," <<
//    s.tnobs << "," <<
//    s.tyi << "," <<
//    s.tvi << "," <<
    s.inx << "," <<
    s.nobs << "," <<
    s.yi << "," <<
    s.sei << "," <<
    s.statistic << "," <<
    s.pvalue << ",";
    for (auto &e : s.effects){
        os << e << ",";
    };
    os << s.side << ",";
    for (auto &hid : s.hHistory) {
        os << hid << ";";
    }
    os << ",";
    os << s.isHacked;
    
    return os;
}


std::string Submission::header(const json &effectslist) {
    
    // FIXME: There is something wrong with this function! For some reason, if I change
    // it, cmake just ignores it and the changes do not reflect to the code!
    
    std::string header{"simid,pid,inx,nobs,yi,sei,statistic,pvalue,"};
    
    for (auto &estimator : effectslist){
        header += estimator;
        header += ",";
    }
    
    header += "side,hids,ish";
    
    return header;
}
