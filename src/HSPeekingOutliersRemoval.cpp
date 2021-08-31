//
// Created by Amir Masoud Abdol on 2020-08-24.
//

#include "HackingStrategy.h"

using namespace sam;

void PeekingOutliersRemoval::perform(Experiment *experiment) {
  spdlog::debug("Peaking Outliers Removal");
  
  for (const auto k : params.multipliers) {
    
    Experiment *copy_of_expr = experiment;
    spdlog::trace("Copying the experiment...");
    
    bool res {true};
    
    /// Removing outliers `n` at a time, for the total of `n_attempts`
    /// It'll stop either when n_attempts are exhausted, or there is no
    /// more observations left to be removed
    for (int t = 0; t < params.n_attempts && res; ++t) {
      
      spdlog::trace("Removing the outliers of the copied experiment...");
      res = OutliersRemoval::removeOutliers(copy_of_expr, params.num, k, 
                                    params.side, params.target,
                                    params.order);
      
      copy_of_expr->recalculateEverything();
      
      if (!params.whether_to_save_cond_defs.empty()) {
        if (whether_to_save_condition(copy_of_expr)) {
          spdlog::trace("Accepting the outlier removal..., replacing the experiment with the accepted copy.");
          experiment = copy_of_expr;
          
          spdlog::trace("{}", *experiment);
          
        } else {
          spdlog::trace("Rejecting the outlier removal..., no improvements found.");
        }
      }
      
      if (!params.stopping_cond_defs.empty()) {
        if (stopping_condition(copy_of_expr)) {
          spdlog::trace("⚠️ Stopping the hacking procedure, stopping condition has been met!");
          return;
        }
      }
      
    }
    
  }
  
  spdlog::trace("{}", (*experiment));
  
}


