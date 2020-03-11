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
        
        id_ = index;
        nobs_ = e.groups_[index].nobs_;
        mean_ = e.groups_[index].mean_;
        var_ = e.groups_[index].var_;
        ses_ = e.groups_[index].ses_;
        stats_ = e.groups_[index].stats_;
        pvalue_ = e.groups_[index].pvalue_;
        effect_ = e.groups_[index].effect_;
        sig_ = e.groups_[index].sig_;
//        is_hacked = e.is_hacked;
//        hHistory = e.hacks_history;
        
        group_ = e[index];
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
    
        record["tnobs"] = std::to_string(tnobs);
        
        std::map<std::string, std::string> g_record = group_;
        record.insert(g_record.begin(), g_record.end());
        
        return record;
    }


}
