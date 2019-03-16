---
parent: API
---

SDOutlierRemoval Class Reference
================================

SDOutlierRemoval

Declaration of Outlier Removal hacking method based on items\' distance
from their sample mean.

`#include <HackingStrategies.h>`

-   **SDOutlierRemoval** (std::string mode, std::string level,
    std::string order, int num, int n\_attempts, int max\_attempts, int
    min\_observations, std::vector\< double \> multipliers)

-   void perform (Experiment \*experiment, DecisionStrategy
    \*decisionStrategy)

    Implementation of Outliers Removal based on an item\'s distance from
    the \\mu.

Detailed Description
--------------------

Declaration of Outlier Removal hacking method based on items\' distance
from their sample mean.

`sd_multiplier` \...

`{ "type": "SD Outlier Removal", "sd_multiplier": d }`

Member Function Documentation
-----------------------------

### perform()

perform

SDOutlierRemoval

SDOutlierRemoval

perform

`void SDOutlierRemoval::perform (Experiment * experiment, DecisionStrategy * decisionStrategy)[virtual]`

Implementation of Outliers Removal based on an item\'s distance from the
\\mu.

The `_sd_multiplier`, d is set at the construction, and it\'ll be used
to check whether a measurement should be removed or not. Any item
satisfying the v \> \|\\mu - d \* \\sigma\| will be removed from the
dataset.

Implements HackingStrategy.

The documentation for this class was generated from the following files:

include/HackingStrategies.h

src/HackingStrategies.cpp
