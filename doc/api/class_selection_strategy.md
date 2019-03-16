---
parent: API
---

SelectionStrategy Class Reference
=================================

SelectionStrategy

Abstract class for Journal\'s selection strategies.

`#include <SelectionStrategies.h>`

-   virtual bool **review** (Submission &s)=0

<!-- -->

-   static SelectionStrategy \* **buildSelectionStrategy** (json
    &config)

Detailed Description
--------------------

Abstract class for Journal\'s selection strategies.

A Journal will decide if a Submission is going to be accepted or
rejected. This decision can be made based on different criteria or
formula. A SelectionStrategy provides an interface for implementing
different selection strategies.

The documentation for this class was generated from the following files:

include/SelectionStrategies.h

src/SelectionStrategies.cpp
