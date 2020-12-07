//
// Created by Amir Masoud Abdol on 2019-01-25.
//

///
/// @defgroup   SelectionStrategies Selection Strategies
/// @brief      List of available Selection Strategies
///
/// Description to come!
///

#ifndef SAMPP_SELECTIONSTRATEGIES_H
#define SAMPP_SELECTIONSTRATEGIES_H

#include "Submission.h"
#include "Utilities.h"
#include <sol/sol.hpp>
#include "Policy.h"

namespace sam {

class Journal;

enum class SelectionMethod {
  PolicyBasedSelection,
  SignificantSelection,
  RandomSelection,
  FreeSelection
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    SelectionMethod,
    {{SelectionMethod::PolicyBasedSelection, "PolicyBasedSelection"},
     {SelectionMethod::SignificantSelection, "SignificantSelection"},
     {SelectionMethod::RandomSelection, "RandomSelection"},
     {SelectionMethod::FreeSelection, "FreeSelection"}
})

///
/// @brief      Abstract class for Journal's selection strategies.
///
/// A Journal will decide if a Submission is going to be accepted or rejected.
/// This decision can be made based on different criteria or formula. A
/// SelectionStrategy provides an interface for implementing different selection
/// strategies.
///
class SelectionStrategy {

protected:
  Distribution dist;

public:
  ///
  /// @brief      Pure deconstructor of the base class
  ///
  virtual ~SelectionStrategy() = 0;
  
  sol::state lua;

  SelectionMethod name;
  SelectionStrategy();

  ///
  /// @brief      Factory method for building a SelectionStrategy
  ///
  /// @param      config  A reference to `json['Journal Parameters']. Usually
  ///                     Researcher::Builder is responsible for passing the
  ///                     object correctly.
  ///
  /// @return     A new SelectionStrategy
  ///
  static std::unique_ptr<SelectionStrategy>
  build(json &selection_straregy_config);

  ///
  /// @brief      Review the Submission and decides if it's going to be accepted
  ///             or rejected. When deriving from SelectionStrategy, `review` is
  ///             the main interface and `Journal` relies on its output
  ///
  /// @param[in]  s     A reference to a Submission
  ///
  /// @return     A boolean indicating whether the Submission should be accepted.
  ///
  virtual bool review(const Submission &s) = 0;
};


///
/// @brief Policy-based Selection Strategy
///
/// Policy-based selection strategy accepts a submission if it passes
/// a criteria specified in `selection_policy_defs`. In addition to the
/// output of selection policy, a submission might get rejected based on
/// journal's acceptance rate, and publication bias rate.
///
///
/// @ingroup  SelectionStrategies
///
///
class PolicyBasedSelection final : public SelectionStrategy {
  
public:
  struct Parameters {
    SelectionMethod name = SelectionMethod::PolicyBasedSelection;
    
    //! Publication Bias Rate
    double pub_bias {0.5};
    
    //! Acceptance Rate
    double pub_chance {0.5};
    
    //! Definition of the selection policy used by Journal to evaluate a given submission
    std::vector<std::string> selection_policy_defs {"sig"};
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PolicyBasedSelection::Parameters, name, pub_bias, pub_chance, selection_policy_defs);
  };
  
  Parameters params;
  PolicyChain selection_policy;
  
  PolicyBasedSelection(Parameters &p) : params{p} {
    selection_policy = PolicyChain(params.selection_policy_defs, lua);
  }
  
  virtual bool review(const Submission &s) override;
};

///
/// @brief      Significant-based Selection Strategy
///
/// Significant-based selection strategy accepts a publication if the given
/// *p*-value is significant. Certain degree of *publication bias*, can be
/// specified. In this case, a Submission has a chance of being published even
/// if the statistics is not significant. Moreover, the SignificantSelection can
/// be tailored toward either positive or negative effect. In this case, the
/// Journal will only accept Submissions with larger or smaller effects.
///
/// @ingroup  SelectionStrategies
///
class SignificantSelection final : public SelectionStrategy {

public:
  ///
  /// A type keeping the parameters of the Significat Seleciton strategy.
  ///
  struct Parameters {
    //! Selection strategy name
    SelectionMethod name = SelectionMethod::SignificantSelection;

    //! The \alpha at which the _selection strategy_ decides the significance
    //! of a publication
    double alpha {0.05};

    //! Publication bias rate
    double pub_bias {0.5};

    //! Indicates the _selection stratgy_'s preference toward positive, `1`,
    //! or negative, `-1` effect. If `0`, Journal doesn't have any preferences.
    int side {1};
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SignificantSelection::Parameters, name, alpha, pub_bias, side);
  };

  Parameters params;

  SignificantSelection(const Parameters &p) : params{p} {};

  virtual bool review(const Submission &s) override;
};

///
/// @brief      Random Selection Strategy
///
/// In this method, Journal does not check any criteria for accepting or
/// rejecting a submission. Each submission has 50% chance of being accepted or
/// not.
///
/// @note       This is technically the SignificantSelection with pub_bias set
/// to 0.
///
/// @ingroup  SelectionStrategies
///
class RandomSelection final : public SelectionStrategy {

public:
  struct Parameters {
    SelectionMethod name = SelectionMethod::RandomSelection;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RandomSelection::Parameters, name);
  };

  Parameters params;

  RandomSelection(const Parameters &p) : params{p} {};

  virtual bool review(const Submission &s) override;
};


///
/// @brief      FreeSelection doesn't pose any restriction on the submission and
///             all submissions will be accepted.
///
///
/// @ingroup  SelectionStrategies
///
class FreeSelection final : public SelectionStrategy {

public:
  struct Parameters {};

  Parameters params;

  FreeSelection(){};

  /// Accepting anything!
  virtual bool review(const Submission &s) override { return true; };
};

} // namespace sam

#endif // SAMPP_SELECTIONSTRATEGIES_H
