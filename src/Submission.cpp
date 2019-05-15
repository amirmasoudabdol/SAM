//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include "Submission.h"

namespace sam {

    Submission::Submission(Experiment& e, const int &index) {
            
       tnobs = e.setup.nobs()[index];

        inx = index;
        nobs = e.measurements[index].size();        // TODO: I think this needs to be generalized
        yi = e.means[index];
        vi = e.vars[index];
        sei = e.ses[index];

        statistic = e.statistics[index];
        pvalue = e.pvalues[index];
        
//        for (auto &estimator : e.effect_strategy){
//            effects.push_back(e.effects[estimator->name][index]);
//        }
        
        effect = e.effects[index];
        
        sig = e.sigs[index];
        
        // CHECK: This will cause problem if I do GroupPooling!
        // BUG: This is also a problem if I'm working with the LatentModel because I'm
        // storing latent means, vars with different names. **This is just not a good idea**.
        // Submission should be self-contained and I shouldn't look into another object
        // FIXME: This is fishy!
        side = std::copysign(1.0, yi - e.setup.means()[index]);
        
        isHacked = e.is_hacked;
        
        hHistory = e.hacks_history;
    };

    std::ostream& operator<<(std::ostream& os, const Submission& s){
    	os <<
        s.simid << "," <<
        s.pubid << "," <<
        s.tnobs << "," <<
        s.inx << "," <<
        s.nobs << "," <<
        s.yi << "," <<
        s.vi << "," <<
        s.sei << "," <<
        s.statistic << "," <<
        s.pvalue << "," <<
        s.effect << ",";
//        for (auto &e : s.effect){
//            os << e << ",";
//        };
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
