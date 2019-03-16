---
parent: API
---

LinearModelStrategy Class Reference
===================================

LinearModelStrategy

`#include <DataGenStrategy.h>`

-   **LinearModelStrategy** (ExperimentSetup &setup)

-   void **genData** (Experiment \*experiment)

-   std::vector\< arma::Row\< double \> \>
    **genNewObservationsForAllGroups** (Experiment \*experiment, int
    n\_new\_obs)

-   arma::Row\< double \> **genNewObservationsFor** (Experiment
    \*experiment, int g, int n\_new\_obs)

Detailed Description
--------------------

The fixed-effect data strategy will produce data from a fixed-effect
model with the given \\mu and sigma.

The documentation for this class was generated from the following files:

include/DataGenStrategy.h

src/DataGenStrategy.cpp
