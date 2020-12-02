//
// Created by Amir Masoud Abdol on 2020-08-24.
//

#include "HackingStrategy.h"

using namespace sam;

void QuestionableRounding::perform(Experiment *experiment) {
  spdlog::debug("Questionable Rounding");
  

  for (int i{experiment->setup.nd()}, d{0}; i < experiment->setup.ng();
       ++i, ++d %= experiment->setup.nd()) {
    
    /// Checking whether the outcome is significant, if not, we check whether
    /// it's close enough to be a hacked, if so, we either set the pvalue
    /// to `alpha` or the difference between pvalue and threshorld
    ///
    if ((experiment->groups_[i].pvalue_ > experiment->test_strategy->alpha()) and
        (experiment->groups_[i].pvalue_ - params.threshold < experiment->test_strategy->alpha())) {
      
      if (params.rounding_method == "diff")
        experiment->groups_[i].pvalue_ = experiment->groups_[i].pvalue_ - params.threshold;
      else if (params.rounding_method == "alpha")
        experiment->groups_[i].pvalue_ = experiment->test_strategy->alpha();
      
    }
      
  }
  
}
