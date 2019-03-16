---
parent: API
---

DataGenStrategy Class Reference
===============================

DataGenStrategy

Abstract class for Data Strategies.

`#include <DataGenStrategy.h>`

-   virtual void **genData** (Experiment \*experiment)=0

-   virtual std::vector\< arma::Row\< double \> \>
    **genNewObservationsForAllGroups** (Experiment \*experiment, int
    n\_new\_obs)=0

-   virtual arma::Row\< double \> **genNewObservationsFor** (Experiment
    \*experiment, int g, int n\_new\_obs)=0

<!-- -->

-   static DataGenStrategy \* **buildDataStrategy** (ExperimentSetup
    &setup)

Detailed Description
--------------------

Abstract class for Data Strategies.

A DataGenStrategy should at least two methods, `genData` and
`genNewObservationForAllGroups`. The former is mainly used to populate a
new Experiment while the latter is being used by some hacking
strategies, e.g. OptionalStopping, where new data --- from the same
population --- is needed.

> **Note**
>
> Each Data Strategy should have access to an instance of
> RandomNumberGenerator. This is usually done by creating a desired
> *random engine* and passing the pointer to the DataGenStrategy.

The documentation for this class was generated from the following files:

include/DataGenStrategy.h

src/DataGenStrategy.cpp
