---
parent: API
---

Experiment Class Reference
==========================

Experiment

Experiment class declaration.

`#include <Experiment.h>`

-   **Experiment** (json &config)

-   **Experiment** (ExperimentSetup &e)

-   void **setTestStrategy** (TestStrategy \*t)

-   void **runTest** ()

-   void **setDataStrategy** (DataGenStrategy \*d)

-   void **initResources** ()

-   void **initExperiment** ()

-   void **generateData** ()

-   void **calculateStatistics** ()

-   void **calculateEffects** ()

-   void **recalculateEverything** ()

-   void **randomize** ()

<!-- -->

-   ExperimentSetup **setup**

-   arma::Row\< int \> **nobs**

-   arma::Row\< double \> **means**

-   arma::Row\< double \> **vars**

-   arma::Row\< double \> **ses**

-   arma::Row\< double \> **statistics**

-   arma::Row\< double \> **pvalues**

-   arma::Row\< double \> **effects**

-   arma::Row\< short \> **sigs**

-   std::vector\< arma::Row\< double \> \> **measurements**

-   bool **latentDesign** = false

-   std::vector\< arma::Row\< double \> \> **items**

-   arma::Row\< double \> **latent\_means**

-   arma::Row\< double \> **latent\_variances**

-   TestStrategy \* **testStrategy**

-   DataGenStrategy \* **dataStrategy**

-   bool isHacked = false

Detailed Description
--------------------

Experiment class declaration.

Member Data Documentation
-------------------------

### isHacked

isHacked

Experiment

Experiment

isHacked

`bool Experiment::isHacked = false`

Indicates if any hacking routine has been applied on the experiment

The documentation for this class was generated from the following files:

include/Experiment.h

src/Experiment.cpp
