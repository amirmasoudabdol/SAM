//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#ifndef SAMPP_DATAGENSTRATEGY_H
#define SAMPP_DATAGENSTRATEGY_H

#include <vector>

class DataGenStrategy{
public:
    DataGenStrategy();
    ~DataGenStrategy();

    virtual std::vector<std::vector<double>> genData();

    // Generating N new data for `dvid` in `gid`
    virtual std::vector<double> genNDataFor(int gid, int dvid, int n);

private:
    int _ngroups;
    int _ndvs;
    std::vector<double> _mean;
    std::vector<double> _sd;
    std::vector<std::vector<double>> _data;

};

class FixedEffectStrategy : DataGenStrategy {
public:

private:

};

#endif //SAMPP_DATAGENSTRATEGY_H
