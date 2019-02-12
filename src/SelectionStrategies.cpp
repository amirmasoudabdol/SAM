//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <SelectionStrategies.h>



//@param s A submission record to be reviewed
//@return A boolean value indicating the acceptance status

/**
 Review the given submission based on its _p_-value and its effect side.
 */
bool SignigicantSelection::review(Submission &s) {
    if (s.pvalue < _alpha && s.side == _side){
        return true;
    }else
        if (_rngEngine->uniform() < _pub_bias) {
            return true;
        }

    return false;
}
