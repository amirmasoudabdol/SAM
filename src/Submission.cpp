//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include "Submission.h"

namespace sam {

    Submission::Submission(Experiment& e, const int &index) {
        
        // This is an ugly hack to solve the GroupPooling problem, I need to move
        // this to the Experiment
        if (index < e.setup.ng()){
            tnobs = e.setup.nobs()[index];
        }else{
            tnobs = 0;
        }
        

        inx = index;
        
        nobs = e.measurements[index].size();        // TODO: I think this needs to be generalized
        mean = e.means[index];
        var = e.vars[index];
        sei = e.ses[index];

        statistic = e.statistics[index];
        pvalue = e.pvalues[index];
        
        effect = e.effects[index];
        
        sig = e.sigs[index];
        
        // CHECK: This will cause problem if I do GroupPooling!
        // BUG: This is also a problem if I'm working with the LatentModel because I'm
        // storing latent means, vars with different names. **This is just not a good idea**.
        // Submission should be self-contained and I shouldn't look into another object
        // FIXME: This is fishy!
        side = std::copysign(1.0, mean - e.setup.means()[index]);
        
        isHacked = e.is_hacked;
        
        hHistory = e.hacks_history;
    };

    std::ostream& operator<<(std::ostream& os, const Submission& s){
    	os <<
        s.simid << ",\t" <<
        s.pubid << ",\t" <<
        s.tnobs << ",\t" <<
        s.inx << ",\t" <<
        s.nobs << ",\t" <<
        s.mean << ",\t" <<
        s.var << ",\t" <<
        s.sei << ",\t" <<
        s.statistic << ",\t" <<
        s.pvalue << ",\t" <<
        s.effect << ",\t";
        os << s.side << ",\t";
        for (auto &hid : s.hHistory) {
            os << hid << ";";
        }
        os << ",\t";
        os << s.isHacked;
        
        return os;
    }
    
    
    std::vector<std::string> Submission::cols() {
         if (!record.empty()){
            for (auto &item : record) {
                columns.push_back(item.first);
            }
         }
        return columns;
    }


//    std::string Submission::header(const json &effectslist) {
//
//        std::string header{"simid,pid,inx,nobs,yi,vi,sei,statistic,pvalue,"};
//
//        for (auto &estimator : effectslist){
//            header += estimator;
//            header += ",";
//        }
//
//        header += "side,hids,ish";
//        
//        return header;
//    }

    Submission::operator std::map<std::string, std::string>() {
            
            record["simid"] = std::to_string(simid);
            record["pubid"] = std::to_string(pubid);
            record["inx"] = std::to_string(inx);
            record["nobs"] = std::to_string(nobs);
            record["yi"] = std::to_string(mean);
            record["vi"] = std::to_string(var);
            record["sei"] = std::to_string(sei);
            record["statistic"] = std::to_string(statistic);
            record["pvalue"] = std::to_string(pvalue);
            record["effect"] = std::to_string(effect);
            record["sig"] = std::to_string(sig);
            record["side"] = std::to_string(side);
            record["isHacked"] = std::to_string(isHacked);
            record["tnobs"] = std::to_string(tnobs);
            
            return record;
        }


}
