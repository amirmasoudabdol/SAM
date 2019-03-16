---
parent: API
---

SignigicantSelection Class Reference
====================================

SignigicantSelection

Significant-based Selection Strategy.

`#include <SelectionStrategies.h>`

-   **SignigicantSelection** (double alpha, double pub\_bias, int side,
    int seed)

-   bool review (Submission &s)

<!-- -->

-   double \_alpha

    The \\alpha at which the *selection strategy* decides the
    significance of a publication.

-   double **\_pub\_bias**

-   int \_side

    Indicates the *selection stratgy*\'s preference toward positive,
    `1`, or negative, `-1` effect. If `0`, Journal doesn\'t have any
    preferences.

-   int **\_seed**

-   RandomNumberGenerator \* **mainRngStream**

Detailed Description
--------------------

Significant-based Selection Strategy.

Significant-based selection strategy accepts a publication if the given
*p*-value is significant. Certain degree of *Publication Bias*, can be
specified. In this case, a Submission has a chance of being published
even if the statistics is not significant. Moreover, the
SignificantSelection can be tailored toward either positive or negative
effect. In this case, the Journal will only accept Submissions with
larger or smaller effects.

Member Function Documentation
-----------------------------

### review()

review

SignigicantSelection

SignigicantSelection

review

`bool SignigicantSelection::review (Submission & s)[virtual]`

Review the given submission based on its *p*-value and its effect side.

Implements SelectionStrategy.

The documentation for this class was generated from the following files:

include/SelectionStrategies.h

src/SelectionStrategies.cpp
