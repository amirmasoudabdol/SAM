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
        nobs = e.groups_[index].nobs_;
        mean = e.groups_[index].mean_;
        var = e.groups_[index].var_;
        sei = e.groups_[index].ses_;
        statistic = e.groups_[index].stats_;
        pvalue = e.groups_[index].pvalue_;
        effect = e.groups_[index].effect_;
        sig = e.groups_[index].sig_;
        is_hacked = e.is_hacked;
        hHistory = e.hacks_history;
    };    
    
    std::vector<std::string> Submission::cols() {
         if (!record.empty()){
            for (auto &item : record) {
                columns.push_back(item.first);
            }
         }
        return columns;
    }

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
            record["side"] = std::to_string(eff_side);
            record["isHacked"] = std::to_string(is_hacked);
            record["tnobs"] = std::to_string(tnobs);
            
            return record;
        }


}
