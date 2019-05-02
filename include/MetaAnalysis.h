//
// Created by Amir Masoud Abdol on 2019-04-25.
//

#ifndef SAMPP_METAANALYSIS_H
#define SAMPP_METAANALYSIS_H

#include <vector>

#include <mlpack/core.hpp>
#include <mlpack/methods/neighbor_search/neighbor_search.hpp>
#include <mlpack/methods/linear_regression/linear_regression.hpp>

//#include <armadillo>

#include "Experiment.h"
#include "Submission.h"

namespace sam {
    
    using namespace std;

	class MetaAnalysis {
    
	public:
//        MetaAnalysis() = default;
        
//        ~MetaAnalysis() = 0;

        virtual arma::vec estimate(vector<Submission> publications) = 0;
	    
	};
    
    
    class FixedEffectEstimator : public MetaAnalysis {
        
    public:
        FixedEffectEstimator() = default;
        
        arma::vec estimate(vector<Submission> publications);
    
    };
    
    class RandomEffectEstimator : public MetaAnalysis {
        
    public:
        RandomEffectEstimator() = default;
        
        arma::vec estimate(vector<Submission> publications);
    };
    

} // namespace sam

#endif //SAMPP_METAANALYSIS_H
