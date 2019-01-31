//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <SelectionStrategies.h>

bool SignigicantSelection::review(Submission &s) {
    if (s.pvalue < _alpha){
        return true;
    }else
        if (_rngEngine->uniform() < _pub_bias) {
            return true;
        }

    return false;
}
