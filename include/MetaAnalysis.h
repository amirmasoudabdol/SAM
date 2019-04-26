//
// Created by Amir Masoud Abdol on 2019-04-25.
//

#ifndef SAMPP_METAANALYSIS_H
#define SAMPP_METAANALYSIS_H

#include <vector>
#include <armadillo>

#include "Experiment.h"
#include "SubmissionRecord.h"

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
