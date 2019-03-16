---
parent: API
---

ImpatientDecisionMaker Class Reference
======================================

ImpatientDecisionMaker

Implementation of an impatient researcher. In this case, the Researcher
will stop as soon as find a significant result and will not continue
exploring other hacking methods in his arsenal.

`#include <DecisionStrategy.h>`

-   **ImpatientDecisionMaker** (DecisionPreference selection\_pref)

-   Submission **selectOutcome** (Experiment &experiment)

-   bool **isPublishable** (const Submission &sub)

-   void **clearPools** ()

-   bool **verdict** (Experiment &experiment, DecisionStage stage)

<!-- -->

-   std::vector\< Submission \> **submissionsPool**

-   std::vector\< Experiment \> **experimentsPool**

Detailed Description
--------------------

Implementation of an impatient researcher. In this case, the Researcher
will stop as soon as find a significant result and will not continue
exploring other hacking methods in his arsenal.

The documentation for this class was generated from the following file:

include/DecisionStrategy.h
