//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include "SubmissionRecord.h"

namespace sam {

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
        s.vi << "," <<
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
        
        std::string header{"simid,pid,inx,nobs,yi,vi,sei,statistic,pvalue,"};
        
        for (auto &estimator : effectslist){
            header += estimator;
            header += ",";
        }
        
        header += "side,hids,ish";
        
        return header;
    }


}