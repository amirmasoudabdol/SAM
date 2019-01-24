//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_EXPERIMENTSETUP_H
#define SAMPP_EXPERIMENTSETUP_H

#include <vector>

class ExperimentSetup {
public:
    ExperimentSetup();
//    ExperimentSetup(int, int, std::vector<double>, std::vector<double>);
    ~ExperimentSetup();

    int ngroups;
    std::vector<std::string> group_names;

    int ndvs;
    std::vector<std::string> dv_names;

    bool _is_correalted = false;    // If true, then we are expecting a matrix.
    std::vector<double> true_means;
    std::vector<double> true_vars;
    std::vector<double> true_cov;
    std::vector<std::vector<double>> cov_matrix;

private:
    
};

#endif //SAMPP_EXPERIMENTSETUP_H
