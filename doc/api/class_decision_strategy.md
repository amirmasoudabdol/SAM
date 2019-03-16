---
parent: API
---

DecisionStrategy Class Reference
================================

DecisionStrategy

Abstract class for different decision strategies.

`#include <DecisionStrategy.h>`

-   virtual \~DecisionStrategy ()=0

-   virtual Submission **selectOutcome** (Experiment &experiment)=0

-   virtual bool **verdict** (Experiment &, DecisionStage)=0

-   Submission \_select\_Outcome (Experiment &experiment)

    Based on the DecisionPreference, it\'ll select the outcome between
    all groups, `ng`. For instance, the MinPvalue deicison prefenrece
    will.

<!-- -->

-   static DecisionStrategy \* **buildDecisionStrategy** (json &config)

<!-- -->

-   DecisionPreference selectionPref

    Indicates researcher\'s selection preference on how he choose the
    outcome variable for submission.

-   bool isStillHacking = true

    If `true`, the Researcher will continue traversing through the
    hacknig methods, otherwise, he/she will stop the hacking and prepare
    the finalSubmission. It will be updated on each call of verdict().
    Basically verdict() decides if the Researcher is happy with the
    submission record or not.

-   int preRegGroup = 0

    Indicates the pre-registered outcome in the case where the
    Researcher prefers the PreRegisteredOutcome.

-   Submission finalSubmission

    This will set to the final submission recrod that the Researcher is
    satisfied about. At the same time, isStillHacking will set to
    `false`.

Detailed Description
--------------------

Abstract class for different decision strategies.

Constructor & Destructor Documentation
--------------------------------------

### \~DecisionStrategy()

\~DecisionStrategy

DecisionStrategy

DecisionStrategy

\~DecisionStrategy

`DecisionStrategy::~DecisionStrategy ( )[pure virtual]`

Pure deconstructor of DecisionStrategy. This is necessary for proper
deconstruction of derived classes.

Member Function Documentation
-----------------------------

### \_select\_Outcome()

\_select\_Outcome

DecisionStrategy

DecisionStrategy

\_select\_Outcome

`Submission DecisionStrategy::_select_Outcome (Experiment & experiment)`

Based on the DecisionPreference, it\'ll select the outcome between all
groups, `ng`. For instance, the MinPvalue deicison prefenrece will.

**Parameters.**

  ------------- --------------------------------------------------------
  experiment    { parameter\_description }

  ------------- --------------------------------------------------------

**Returns:.**

A copy of the selected outcome

The documentation for this class was generated from the following files:

include/DecisionStrategy.h

src/DecisionStrategy.cpp
