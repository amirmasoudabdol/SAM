---
parent: API
---

TTest Class Reference
=====================

TTest

`#include <TestStrategy.h>`

-   **TTest** (TestSide side, double alpha)

-   void **run** (Experiment \*experiment)

Detailed Description
--------------------

Declration of t-test.

The `run()` method will check the significance of the difference between
two groups. In the current setup, every `experiment->means` is
considered an effect size between a treatment group and a control group
with the mean of zero. Therefore, computing the t-statistics and
computing the p-value would be sufficient. This is technically an
implementation of one sample t-test.

The documentation for this class was generated from the following files:

include/TestStrategy.h

src/TestStrategy.cpp
