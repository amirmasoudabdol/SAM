//
// Created by Amir Masoud Abdol on 2019-05-23.
//

#ifndef SAMPP_HACKINGSTRATEGYTYPES_H
#define SAMPP_HACKINGSTRATEGYTYPES_H

#include "nlohmann/json.hpp"

namespace sam {

///
/// Hacking Strategy methods.
///
enum class HackingMethod {
  OptionalStopping,
  OutliersRemoval,
  GroupPooling,
  ConditionDropping,
  QuestionableRounding,
  PeekingOutliersRemoval,
  FalsifyingData,
  FabricatingData,
  StoppingDataCollection,
  OptionalDropping,
  NoHack = -1   /// @todo I probably don't need this!
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    HackingMethod,
    {{HackingMethod::OptionalStopping, "OptionalStopping"},
     {HackingMethod::OutliersRemoval, "OutliersRemoval"},
     {HackingMethod::GroupPooling, "GroupPooling"},
     {HackingMethod::ConditionDropping, "ConditionDropping"},
     {HackingMethod::QuestionableRounding, "QuestionableRounding"},
     {HackingMethod::PeekingOutliersRemoval, "PeekingOutliersRemoval"},
     {HackingMethod::FalsifyingData, "FalsifyingData"},
     {HackingMethod::FabricatingData, "FabricatingData"},
     {HackingMethod::StoppingDataCollection, "StoppingDataCollection"},
     {HackingMethod::OptionalDropping, "OptionalDropping"},
     {HackingMethod::NoHack, "NoHack"}})

///
/// HackingStage indicates the stage where the hacking is being performed on the
/// Experiment. Each method will be assigned a value, and Researcher can apply
/// different hacking methods in different stages.
///
enum class HackingStage : int {
  Setup = 0,
  DataCollection,
  PreProcessing,
  PostProcessing,
  Reporting
};

NLOHMANN_JSON_SERIALIZE_ENUM(
   HackingStage,
   {{HackingStage::Setup, "Setup"},
    {HackingStage::DataCollection, "DataCollection"},
    {HackingStage::PreProcessing, "PreProcessing"},
    {HackingStage::PostProcessing, "PostProcessing"},
    {HackingStage::Reporting, "Reporting"}})


enum class HackingTarget : int {
  Control = 0,
  Treatment,
  Both
};

NLOHMANN_JSON_SERIALIZE_ENUM( HackingTarget,
                             {{HackingTarget::Control, "Control"},
                              {HackingTarget::Treatment, "Treatment"},
                              {HackingTarget::Both, "Both"}})

} // namespace sam

#endif // SAMPP_HACKINGSTRATEGYTYPES_H
