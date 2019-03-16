---
parent: API
---

OptionalStopping Class Reference
================================

OptionalStopping

Declartion of OptionalStopping hacking strategy.

`#include <HackingStrategies.h>`

-   **OptionalStopping** (std::string mode, std::string level, int num,
    int n\_attempts, int max\_attempts)

-   void perform (Experiment \*experiment, DecisionStrategy
    \*decisionStrategy)

    Implementation of optional stopping.

Detailed Description
--------------------

Declartion of OptionalStopping hacking strategy.

In order to apply the OptionalStopping, use the following format in the
JSON config file where `n` specify number of new observations that is
going to be added to the experiment in each trial, `m`.

`{    "type": "Optional Stopping",    "size": n,    "attempts": m }`

Member Function Documentation
-----------------------------

### perform()

perform

OptionalStopping

OptionalStopping

perform

`void OptionalStopping::perform (Experiment * experiment, DecisionStrategy * decisionStrategy)[virtual]`

Implementation of optional stopping.

This will use two parameters set at construction of the OptionalStopping
class, `_n_trials` and `_n_new_obs` for every trial, the routine will
add `_n_new_obs` to all groups, recalculate the statistics, and run the
test. It will then select an outcome based on researcher\'s preference
and check it\'s significance. If the results is significant, it\'ll not
make a new attempt to add more data, and will return to the hack()
routine.

Implements HackingStrategy.

The documentation for this class was generated from the following files:

include/HackingStrategies.h

src/HackingStrategies.cpp
