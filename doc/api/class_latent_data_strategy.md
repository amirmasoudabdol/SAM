---
parent: API
---

LatentDataStrategy Class Reference
==================================

LatentDataStrategy

`#include <DataGenStrategy.h>`

-   **LatentDataStrategy** (ExperimentSetup &setup)

-   void genData (Experiment \*experiment)

-   std::vector\< arma::Row\< double \> \>
    **genNewObservationsForAllGroups** (Experiment \*experiment, int
    n\_new\_obs)

-   arma::Row\< double \> **genNewObservationsFor** (Experiment
    \*experiment, int g, int n\_new\_obs)

Detailed Description
--------------------

A Data Strategy for constructing a general Structural Equaiton Model.

> **Note**
>
> LatentDataStrategy will generate individual items, therefore it might
> be slower than other models.

Member Function Documentation
-----------------------------

### genData()

genData

LatentDataStrategy

LatentDataStrategy

genData

`void LatentDataStrategy::genData (Experiment * experiment)[virtual]`

Construct a structural equation model based on the given paramters
specified in the ExperimentSteup. Beside `experiment->measurements`,
LatentDataStrategy populates the `experiment->items` as well.

TODO: Link to the illustration that explain the model

Implements DataGenStrategy.

The documentation for this class was generated from the following files:

include/DataGenStrategy.h

src/DataGenStrategy.cpp
