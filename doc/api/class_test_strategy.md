---
parent: API
---

TestStrategy Class Reference
============================

TestStrategy

Abstract class for defining test strategies.

`#include <TestStrategy.h>`

-   virtual void **run** (Experiment \*experiment)=0

<!-- -->

-   static TestStrategy \* **buildTestStrategy** (json &config)

Detailed Description
--------------------

Abstract class for defining test strategies.

Statistical test strategies will investigate if there is a meaningful
difference between means of two samples. Every test strategy should
provide a `run()` method. The `run()` method will accept a pointer to
the experiment and update necessary variables, e.g., *statistics* &
*p-value*.

The documentation for this class was generated from the following files:

include/TestStrategy.h

src/TestStrategy.cpp
