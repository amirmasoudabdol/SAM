//
// Created by Amir Masoud Abdol on 2019-05-23.
//

#ifndef SAMPP_HACKINGSTRATEGYTYPES_H
#define SAMPP_HACKINGSTRATEGYTYPES_H

/**
 * Hacking Strategy methods.
 */
enum class HackingMethod {
    OptionalStopping = 0,
    SDOutlierRemoval,
    GroupPooling,
    ConditionDropping,
    NoHack
};

/*
 HackingStage indicates the stage where the hacking is being performed on
 the Experiment. Each method will be assigned a value, and Researcher can
 apply different hacking methods in different stages.
 */
enum class HackingStage {
    Setup,
    DataCollection,
    DataProcessing,
    Reporting
};

struct OptionalStoppingParameters {
    HackingMethod name = HackingMethod::OptionalStopping;
    int num;
};

struct SDOutliersRemovalParameters {
    HackingMethod name = HackingMethod::SDOutlierRemoval;
    int num;
};

struct GroupPoolingParameters {
    HackingMethod name = HackingMethod::GroupPooling;
    int num;
};

struct HackingStrategyParameters {
    
    //! Arbitrary ID to keep track of number of hacking methods
    int hid = 0;
    
    HackingMethod name;

    OptionalStoppingParameters osp;
    SDOutliersRemovalParameters orp;
    GroupPoolingParameters gpp;
    
    HackingStage stage = HackingStage::DataProcessing;
    
    //! Defensibility of the method
    //! This is a based on the survey results where researchers have been
    //! asked to rate the defensibility of different QRPs.
    //! Defensitbility of 0 indicates that the method is completely
    //! frown upon and shouldn't be used, while defensibility of 1.
    //! means it's a valid pratice and it's not going to be preseved
    //! as a hacking method.
    double defensibility = 1.;
};

#endif //SAMPP_HACKINGSTRATEGYTYPES_H
